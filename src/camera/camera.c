/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): 
 * Purpose: Selecting targets, camera settings, capturing images, and providing
 *          an interface to each camera.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include "camera_control.h"
#include "sanity_camera.h"
#include "guiding_camera.h"
#include "nir_camera.h"

int init_camera( void ){

    /* init whatever in this module */

    int res[4];

    res[0] = init_camera_control();
    res[1] = init_sanity_camera();
    res[2] = init_guiding_camera();
    res[3] = init_nir_camera();

    if( res[0] != SUCCESS ||
        res[1] != SUCCESS ||
        res[2] != SUCCESS ||
        res[3] != SUCCESS
        ){
        return FAILURE;
    }
    return SUCCESS;
}
