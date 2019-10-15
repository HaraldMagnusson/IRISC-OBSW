/* -----------------------------------------------------------------------------
 * Component Name: Temperature Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll all thermometers for the current temperature status.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"

static void* temp_poller_thread(void* args);
static void active_m(void);

static int fd_i2c;

int init_temperature_poller(void* args){

    fd_i2c = open("/dev/i2c-1", O_RDWR);
    if(fd_i2c == -1){
        logging(ERROR, "Temp_poller", "Failed to open i2c-1 device: %m");
        return errno;
    }

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
