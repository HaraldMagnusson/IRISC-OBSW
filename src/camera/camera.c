/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): 
 * Purpose: Selecting targets, camera settings, capturing images, and providing
 *          an interface to each camera.
 * -----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>

#include "global_utils.h"

#include "camera_control.h"
#include "sanity_camera.h"
#include "guiding_camera.h"
#include "nir_camera.h"

#define MODULE_COUNT 4

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"camera_control", &init_camera_control},
    {"sanity_camera", &init_sanity_camera},
    {"guiding_camera", &init_guiding_camera},
    {"nir_camera", &init_nir_camera}
};

int init_camera( void ){

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if( ret != SUCCESS ){
        return ret;
    }

    return SUCCESS;
}
