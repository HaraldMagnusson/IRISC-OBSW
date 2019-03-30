/* -----------------------------------------------------------------------------
 * Component Name: Watchdog
 * Author(s): William Eriksson
 * Purpose: Initialise and regularly reset the watchdog.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "global_utils.h"

/* Filepath for watchdog */
#ifdef __arm__
    #define WATCHDOG_DIR "/dev/watchdog"
#else
    #define WATCHDOG_DIR "/tmp/watchdog"
#endif

/* Period (in seconds) for petting watchdog */
#define WATCHDOG_WAIT 10

static int fd_watchdog;
static pthread_t thread_watchdog;
static pthread_attr_t thread_attr;
static struct timespec wake_time;
static struct sched_param param;
static int ret;

static void* thread_func( void*);

int init_watchdog( void ){

    fd_watchdog = open(WATCHDOG_DIR, O_WRONLY);
    if( fd_watchdog == -1 ){
        fprintf(stderr,
            "Failed open for watchdog component. "
            "Return value: %d, %s\n", errno, strerror(errno));
        return errno;
    }

    ret = pthread_attr_init( &thread_attr );
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_init for watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setstacksize of watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    param.sched_priority = 50;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_create(&thread_watchdog, &thread_attr, thread_func, NULL);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_create of watchdog component. "
            "Return value: %d\n", ret);
        return ret;
    }

    return SUCCESS;
}

static void* thread_func( void* param){
    /* Thread for petting watchdog */

    clock_gettime(CLOCK_MONOTONIC, &wake_time);

    while(1){
        write(fd_watchdog, "w", 1);

        wake_time.tv_sec += WATCHDOG_WAIT;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
    }

    return NULL;
} 

int stop_watchdog( void ){
    /* Stops thread_watchdog and disables the watchdog */

    pthread_kill(thread_watchdog, SIGTERM);

    /* Writing V prepares the watchdog for closing */
    write(fd_watchdog, "V", 1);
    close(fd_watchdog);

    return SUCCESS;
}
