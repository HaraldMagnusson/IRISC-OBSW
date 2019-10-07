/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): Harald Magnusson
 * Purpose: Selecting targets, camera settings, capturing images, and providing
 *          an interface to each camera.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "global_utils.h"

#include "sanity_camera.h"
#include "guiding_camera.h"
#include "nir_camera.h"

#define MODULE_COUNT 3

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"sanity_camera", &init_sanity_camera},
    {"guiding_camera", &init_guiding_camera},
    {"nir_camera", &init_nir_camera}
};

int init_camera(void* args){

    /* init whatever in this module */
    return init_submodules(init_sequence, MODULE_COUNT);
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
 *      EREMOTEIO: starting exposure failed
 *      EIO: setting camera control values failed
 *      ENODEV: Camera not connected
 */
int expose_guiding(int exp, int gain){
    return expose_guiding_local(exp, gain);
}

/* save_img_guiding:
 * save_img_guiding will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 *
 * input:
 *      fn: filename to save image as
 *
 * return:
 *      SUCCESS: operation is successful
 *      EXP_NOT_READY: exposure still ongoing, wait a bit and call again
 *      EXP_FAILED: exposure failed and must be retried
 *      FAILURE: saving the image failed, log written to stderr
 *      EPERM: calling save_img beore starting exposure
 *      ENOMEM: no memory available for image buffer
 *      EIO: failed to fetch data from camera
 *      ENODEV: camera disconnected
 *
 */
int save_img_guiding(char* fn){
    return save_img_guiding_local(fn);
}

/* abort_exp_guiding:
 * Abort an ongoing exposure of the guiding camera and save the image.
 *
 * input:
 *      fn: filename to save image as
 *
 * return:
 *      SUCCESS: operation is successful
 *      EXP_FAILED: exposure failed and must be retried
 *      FAILURE: saving the image failed, log written to stderr
 *      EPERM: calling save_img beore starting exposure
 *      ENOMEM: no memory available for image buffer
 *      EIO: failed to fetch data from camera
 *      ENODEV: camera disconnected
 */
int abort_exp_guiding(char* fn){
    return abort_exp_guiding_local(fn);
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
 *      EREMOTEIO: starting exposure failed
 *      EIO: setting camera control values failed
 *      ENODEV: Camera not connected
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
 *      SUCCESS: operation is successful
 *      EXP_NOT_READY: exposure still ongoing, wait a bit and call again
 *      EXP_FAILED: exposure failed and must be retried
 *      FAILURE: saving the image failed, log written to stderr
 *      EPERM: calling save_img beore starting exposure
 *      ENOMEM: no memory available for image buffer
 *      EIO: failed to fetch data from camera
 *      ENODEV: camera disconnected
 *
 */
int save_img_nir(void){
    return save_img_nir_local();
}

/* abort_exp_nir:
 * Abort an ongoing exposure of the NIR camera and save the image.
 *
 * input:
 *      fn: filename to save image as
 *
 * return:
 *      SUCCESS: operation is successful
 *      EXP_FAILED: exposure failed and must be retried
 *      FAILURE: saving the image failed, log written to stderr
 *      EPERM: calling save_img beore starting exposure
 *      ENOMEM: no memory available for image buffer
 *      EIO: failed to fetch data from camera
 *      ENODEV: camera disconnected
 */
int abort_exp_nir(char* fn){
    return abort_exp_nir_local(fn);
}
