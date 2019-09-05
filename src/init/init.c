/* -----------------------------------------------------------------------------
 * Component Name: Init
 * Author(s): Harald Magnusson
 * Purpose: Called by OS. Initialise the entire system and start threads.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>

#include "camera.h"
#include "command.h"
#include "data_storage.h"
#include "e_link.h"
#include "global_utils.h"
#include "gpio.h"
#include "img_processing.h"
#include "mode.h"
#include "sensors.h"
#include "telemetry.h"
#include "thermal.h"
#include "tracking.h"
#include "watchdog.h"

#include "star_tracker_caller.h"

/* not including init */
#define MODULE_COUNT 13

static int ret;
static struct sigaction sa;

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"watchdog", &init_watchdog},
    {"gpio", &init_gpio},
    {"camera", &init_camera},
    {"command", &init_command},
    {"data_storage", &init_data_storage},
    {"e_link", &init_elink},
    {"global_utils", &init_global_utils},
    {"img_processing", &init_img_processing},
    {"mode", &init_mode},
    {"sensors", &init_sensors},
    {"telemetry", &init_telemetry},
    {"thermal", &init_thermal},
    {"tracking", &init_tracking}
};

static void sigint_handler(int signum){
    write(STDOUT_FILENO, "\nSIGINT caught, exiting\n", 24);
    stop_watchdog();
    gpio_unexport(GYRO_TRIG_PIN);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]){

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* redirect stderr to a log file */
    /* freopen("../test.log", "w", stderr); */

    ret = mlockall(MCL_CURRENT|MCL_FUTURE);
    if( ret != 0 ){
        logging(ERROR, "INIT",
            "init: Failed mlockall. Return value: %d, %s", errno, strerror(errno));
        //return FAILURE;
    }

    int count = 0;
    for(int i=0; i<MODULE_COUNT; ++i){
        ret = init_sequence[i].init();
        if( ret == SUCCESS ){
            logging(INFO, "INIT", "Module \"%s\" initialised successfully.",
                init_sequence[i].name);
        } else if( ret == FAILURE ){
            logging(ERROR, "INIT", "Module \"%s\" FAILED TO INITIALISE, return value: %d",
                init_sequence[i].name, ret);
            ++count;
        } else {
            logging(ERROR, "INIT", "Module \"%s\" FAILED TO INITIALISE, return value: %d, %s",
                init_sequence[i].name, ret, strerror(ret));
            ++count;
        }
    }

    logging(INFO, "INIT",
        "A total of %d modules initialised successfully and %d failed.",
        MODULE_COUNT-count, count);

    if(count != 0){
        return FAILURE;
    }

    struct timespec samp_0, samp, diff;

    while(1){
        float hax[4];
        clock_gettime(CLOCK_MONOTONIC, &samp_0);
        iriscTetra(hax);
        clock_gettime(CLOCK_MONOTONIC, &samp);

        diff.tv_sec = samp.tv_sec - samp_0.tv_sec;
        diff.tv_nsec = samp.tv_nsec - samp_0.tv_nsec;
        if(diff.tv_nsec < 0){
            diff.tv_sec--;
            diff.tv_nsec += 1000000000;
        }
        logging(DEBUG, "Star Tracker", "Star tracker sample time: %ld.%09ld s",
                diff.tv_sec, diff.tv_nsec);
        if(hax[3] == 0){
            logging(ERROR, "Star Tracker", "Star tracker error");
            return FAILURE;
        }
        for(int ii=0; ii<4; ++ii){
            logging(DEBUG, "Star Tracker", "%f\n", hax[ii]);
        }

        sleep(10);
    }




    return SUCCESS;
}
