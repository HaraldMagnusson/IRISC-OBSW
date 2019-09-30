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
#ifdef __aarch64__
    #define WATCHDOG_DIR "/dev/watchdog"
#elif defined(__arm__)
    #define WATCHDOG_DIR "/dev/watchdog"
#else
    #define WATCHDOG_DIR "/tmp/watchdog"
#endif

/* Period (in seconds) for petting watchdog */
#define WATCHDOG_WAIT 10

static int fd_watchdog;
static struct timespec wake_time;

static void* thread_func( void*);

int init_watchdog(void* args){

    fd_watchdog = open(WATCHDOG_DIR, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
    if( fd_watchdog == -1 ){
        fprintf(stderr,
            "Failed open for watchdog component. "
            "Return value: %d, %s\n", errno, strerror(errno));
        return errno;
    }

    return create_thread("watchdog", thread_func, 10);
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
    /* Disables the watchdog */

    /* Writing V prepares the watchdog for closing */
    write(fd_watchdog, "V", 1);
    close(fd_watchdog);

    return SUCCESS;
}
