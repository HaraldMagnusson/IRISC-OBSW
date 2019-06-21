/* -----------------------------------------------------------------------------
 * Component Name: NIR Camera
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface to the NIR camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include "camera_utils.h"
#include "nir_camera.h"

ASI_CAMERA_INFO cam_info;

/* init_nir_camera:
 * Set up and initialise the nir camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_nir_camera( void ){

    int ret = cam_setup(&cam_info, 'n');
    if(ret != SUCCESS){
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
 *      FAILURE: starting exposure failed, log written to stderr
 */
int expose_nir_local(int exp, int gain){
    return expose(cam_info.CameraID, exp, gain);
}

/* save_img_nir:
 * save_img_nir will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from
 * the camera and saved.
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
int save_img_nir_local(void){
    return save_img(&cam_info);
}
