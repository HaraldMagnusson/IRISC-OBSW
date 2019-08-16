/* -----------------------------------------------------------------------------
 * Component Name: Sensor Poller
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Poll sensors and update storage components.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"

#include "gps_poller.h"
#include "encoder_poller.h"
#include "gyroscope_poller.h"
#include "star_tracker_poller.h"
#include "temperature_poller.h"

#define MODULE_COUNT 5

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"encoder_poller", &init_encoder_poller},
    {"gps_poller", &init_gps_poller},
    {"gyroscope_poller", &init_gyroscope_poller},
    {"star_tracker_poller", &init_star_tracker_poller},
    {"temperature_poller", &init_temperature_poller}
};

int init_sensor_poller( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
