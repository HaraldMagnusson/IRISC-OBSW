/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): Harald Magnusson
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

/* expose_guiding:
 * Start an exposure of the guiding camera. Call save_img to store store
 * image after exposure
 *
 * input:
 *      exp: the exposure time in microseconds
 *      gain: the sensor gain
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: starting exposure failed, log written to stderr
 */
int expose_guiding(int exp, int gain){
    return expose_guiding_local(exp, gain);
}

/* save_img_guiding:
 * save_img_guiding will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 *
 * return:
 *      SUCCESS:       operation is successful
 *      EXP_NOT_READY: exposure still ongoing, wait a bit and call again
 *      EXP_FAILED:    exposure failed and must be retried
 *      FAILURE:       storing the image failed, log written to stderr
 *
 * TODO:
 *      1. fix system for filenames
 *      2. fix .fit header
 */
int save_img_guiding(char* fn){
    return save_img_guiding_local(fn);
}

/* expose_nir:
 * Start an exposure of the nir camera. Call save_img to store store
 * image after exposure
 *
 * input:
 *      exp: the exposure time in microseconds
 *      gain: the sensor gain
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: starting exposure failed, log written to stderr
 */
int expose_nir(int exp, int gain){
    return expose_nir_local(exp, gain);
}

/* save_img_nir:
 * save_img_nir will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 *
 * return:
 *      SUCCESS:       operation is successful
 *      EXP_NOT_READY: exposure still ongoing, wait a bit and call again
 *      EXP_FAILED:    exposure failed and must be retried
 *      FAILURE:       storing the image failed, log written to stderr
 *
 * TODO:
 *      1. fix system for filenames
 *      2. fix .fit header
 */
int save_img_nir(char* fn){
    return save_img_nir_local(fn);
}
