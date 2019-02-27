/* -----------------------------------------------------------------------------
 * Component Name: Watchdog
 * Author(s): William Eriksson
 * Purpose: Initialise and regularly reset the watchdog.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>

static FILE * fp_watchdog;
static pthread_t thread_watchdog;
static pthread_attr_t thread_attr;
static struct timespec wake_time;
static struct sched_param param;
static int res;

static void* thread_func( void*);

int init_watchdog( void ){

    fp_watchdog = fopen("filepath here", "w");
    fprintf(fp_watchdog, "v");

    res = mlockall(MCL_CURRENT|MCL_FUTURE);
    if( res != 0 ){
        fprintf(stderr,
            "Failed mlockall for watchdog component. "
            "Return value: %d\n", errno);
        return FAILURE;
    }

    res = pthread_attr_init( &thread_attr );
    if( res != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_init for watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }

    res = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN);
    if( res != 0 ){
        fprintf(stderr, 
            "Failed pthread_attr_setstacksize of watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }

    res = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if( res != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }

    param.sched_priority = 50;
    res = pthread_attr_setschedparam(&thread_attr, &param);
    if( res != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }

    res = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( res != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }

    res = pthread_create(&thread_watchdog, &thread_attr, thread_func, NULL);
    if( res != 0 ){
        fprintf(stderr,
            "Failed pthread_create of watchdog component. "
            "Return value: %d\n", res);
        return FAILURE;
    }    

    return SUCCESS;
}

static void* thread_func( void* param){

    clock_gettime(CLOCK_MONOTONIC, &wake_time);

    while(1){
        fprintf(fp_wathcdog, "v");

        wake_time.tv_sec += 10;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
    }
}