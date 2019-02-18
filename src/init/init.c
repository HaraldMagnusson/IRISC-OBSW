/* -----------------------------------------------------------------------------
 * Component Name: Init
 * Author(s): Harald Magnusson
 * Purpose: Called by OS. Initialise the entire system and start threads.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>

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

int main(int argc, char const *argv[]){

    if(argc > 1){
        printf("argument received\n");
        printf("argument: %s\n", argv[1]);
    }

    static int res[MODULE_COUNT];

    res[GLOBAL_UTILS] = init_global_utils();
    res[WATCHDOG] = init_watchdog();
    res[DATA_STORAGE] = init_data_storage();
    res[MODE] = init_mode();

    res[I2C] = init_i2c();
    res[SPI] = init_spi();
    res[SENSORS] = init_sensors();
    res[THERMAL] = init_thermal();

    res[E_LINK] = init_elink();
    res[TELEMETRY] = init_telemetry();
    res[COMMAND] = init_command();

    res[CAMERA] = init_camera();
    res[TRACKING] = init_tracking();
    res[IMG_PROCESSING] = init_img_processing();

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

    return SUCCESS;
}
