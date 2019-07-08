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

#include "camera.h"
#include "command.h"
#include "data_storage.h"
#include "e_link.h"
#include "global_utils.h"
#include "i2c.h"
#include "img_processing.h"
#include "mode.h"
#include "sensors.h"
#include "spi.h"
#include "telemetry.h"
#include "thermal.h"
#include "control_sys.h"
#include "watchdog.h"

/* not including init */
#define MODULE_COUNT 14

static int ret;
static struct sigaction sa;

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"watchdog", &init_watchdog},
    {"camera", &init_camera},
    {"command", &init_command},
    {"data_storage", &init_data_storage},
    {"e_link", &init_elink},
    {"global_utils", &init_global_utils},
    {"i2c", &init_i2c},
    {"img_processing", &init_img_processing},
    {"mode", &init_mode},
    {"sensors", &init_sensors},
    {"spi", &init_spi},
    {"telemetry", &init_telemetry},
    {"thermal", &init_thermal},
    {"control_sys", &init_control_sys}
};

static void sigint_handler(int signum){
    write(STDOUT_FILENO, "\nSIGINT caught, exiting\n", 24);
    stop_watchdog();
    _exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]){

    sa.sa_handler = sigint_handler;
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

    while(1){
        sleep(1000);
    }

    return SUCCESS;
}
