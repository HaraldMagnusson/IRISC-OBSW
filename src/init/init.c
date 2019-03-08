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
#include "tracking.h"
#include "watchdog.h"

/* not including init */
#define MODULE_COUNT 14

#define CAMERA 0
#define COMMAND 1
#define DATA_STORAGE 2
#define E_LINK 3
#define GLOBAL_UTILS 4
#define I2C 5
#define IMG_PROCESSING 6
#define MODE 7
#define SENSORS 8
#define SPI 9
#define TELEMETRY 10
#define THERMAL 11
#define TRACKING 12
#define WATCHDOG 13

static const char module_arr[MODULE_COUNT][15] =   {"camera",
                                                    "command",
                                                    "data_storage",
                                                    "e_link",
                                                    "global_utils",
                                                    "i2c",
                                                    "img_processing",
                                                    "mode",
                                                    "sensors",
                                                    "spi",
                                                    "telemetry",
                                                    "thermal",
                                                    "tracking",
                                                    "watchdog"};

static struct sigaction sa;

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

    int res[MODULE_COUNT];

    /* ordered init list */
    res[WATCHDOG] = init_watchdog();

    /* unordered init list */
    res[CAMERA] = init_camera();
    res[COMMAND] = init_command();
    res[DATA_STORAGE] = init_data_storage();
    res[E_LINK] = init_elink();
    res[GLOBAL_UTILS] = init_global_utils();
    res[I2C] = init_i2c();
    res[IMG_PROCESSING] = init_img_processing();
    res[MODE] = init_mode();
    res[SENSORS] = init_sensors();
    res[SPI] = init_spi();
    res[TELEMETRY] = init_telemetry();
    res[THERMAL] = init_thermal();
    res[TRACKING] = init_tracking();

    int count = 0;
    for(int i=0; i<MODULE_COUNT; i++){
        if( res[i] != SUCCESS ){
            fprintf(stderr,
                "The component %s failed to initialise.\n",
                module_arr[i]);
            count++;
        }
    }

    fprintf(stdout,
        "\nA total of %d modules initialised successfully and %d failed.\n\n",
        MODULE_COUNT-count, count);

    if(count != 0){
        return FAILURE;
    }

    while(1){}

    return SUCCESS;
}
