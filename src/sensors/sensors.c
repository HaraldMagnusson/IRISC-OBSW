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
#include "orientation.h"
#include "sensor_poller.h"
#include "sun.h"
#include "temperature.h"

#define MODULE_COUNT 4

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"orientation", &init_orientation},
    {"sensor_poller", &init_sensor_poller},
    {"sun", &init_sun},
    {"temperature", &init_temperature}
};

int init_sensors( void ){

    /* init whatever in this module */
    init_submodules(init_sequence, MODULE_COUNT);

    return SUCCESS;
}
