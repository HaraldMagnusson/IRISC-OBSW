/* -----------------------------------------------------------------------------
 * Component Name: Sensor Poller
 * Parent Component: Sensors
 * Author(s): 
 * Purpose: Poll sensors and update storage components.
 *
 * -----------------------------------------------------------------------------
 */

#include "gps_poller.h"
#include "global_utils.h"

#define MODULE_COUNT 1

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"gps_poller", &init_gps_poller}
};

int init_sensor_poller( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
