/* -----------------------------------------------------------------------------
 * Component Name: Sensors
 * Author(s): 
 * Purpose: Poll sensors and store the latest data. Provide an interface for
 *          other components to read the data.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "global_utils.h"
#include "encoder.h"
#include "gps.h"
#include "gyroscope.h"
#include "sensor_poller.h"
#include "star_tracker.h"
#include "temperature.h"

#define MODULE_COUNT 6

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"encoder", &init_encoder},
    {"gps", &init_gps},
    {"gyroscope", &init_gyroscope},
    {"star_tracker", &init_star_tracker},
    {"temperature", &init_temperature},
    {"sensor_poller", &init_sensor_poller},
};

int init_sensors( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
