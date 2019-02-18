/* -----------------------------------------------------------------------------
 * Component Name: Sensors
 * Author(s): 
 * Purpose: Poll sensors and store the latest data. Provide an interface for
 *          other components to read the data.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>

#include "global_utils.h"
#include "orientation.h"
#include "sensor_poller.h"
#include "sun.h"
#include "temperature.h"

int init_sensors( void ){

    /* init whatever in this module */

    int res[4];

    res[0] = init_orientation();
    res[1] = init_sensor_poller();
    res[2] = init_sun();
    res[3] = init_temperature();

    if( res[0] != SUCCESS ||
        res[1] != SUCCESS ||
        res[2] != SUCCESS ||
        res[3] != SUCCESS
        ){
        return FAILURE;
    }
    return SUCCESS;
}
