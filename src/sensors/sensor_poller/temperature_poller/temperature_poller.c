/* -----------------------------------------------------------------------------
 * Component Name: Temperature Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll all thermometers for the current temperature status.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "global_utils.h"
#include "temperature_poller.h"

static void* temp_poller_thread(void* args);
static void active_m(void);

int init_temperature_poller(void* args){

    return create_thread("temp_poller", temp_poller_thread, 20);
}

static void* temp_poller_thread(void* args){

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    while(1){
        active_m();

        wake.tv_sec += TEMP_SAMPLE_TIME;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);

    }

    return NULL;
}

static void active_m(void){

}
