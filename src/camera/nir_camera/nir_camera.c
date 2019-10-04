/* -----------------------------------------------------------------------------
 * Component Name: NIR Camera
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface to the NIR camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#include <errno.h>
#include <stdlib.h>

#include "global_utils.h"
#include "camera_utils.h"

static ASI_CAMERA_INFO cam_info;

/* init_nir_camera:
 * Set up and initialise the nir camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_nir_camera(void* args){

    int ret = cam_setup(&cam_info, 'n');
    if(ret == ENODEV){
        logging(ERROR, "INIT", "NIR camera not connected");
    }
    else if(ret != SUCCESS){
        return FAILURE;
    }
    return SUCCESS;
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
int expose_nir_local(int exp, int gain){
    return expose(cam_info.CameraID, exp, gain, "NIR");
}

/* save_img_nir:
 * save_img_nir will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from
 * the camera and saved.
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
int save_img_nir_local(char* fn){
    return save_img(&cam_info, fn, "NIR", NULL);
}

/* abort_exp_nir_local:
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
int abort_exp_nir_local(char* fn){
    return abort_exp(&cam_info, fn, "NIR");
}
