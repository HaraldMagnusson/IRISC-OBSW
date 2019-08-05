/* -----------------------------------------------------------------------------
 * Component Name: Guiding Camera
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface to the guiding camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#include <errno.h>

#include "global_utils.h"
#include "camera_utils.h"

static ASI_CAMERA_INFO cam_info;

/* init_guiding_camera:
 * Set up and initialise the guiding camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_guiding_camera( void ){

    int ret = cam_setup(&cam_info, 'g');
    if(ret == ENODEV){
        logging(ERROR, "INIT", "Guiding camera not connected");
    } else if(ret != SUCCESS){
        return FAILURE;
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
 *      EREMOTEIO: starting exposure failed
 *      EIO: setting camera control values failed
 *      ENODEV: Camera not connected
 */
int expose_guiding_local(int exp, int gain){
    return expose(cam_info.CameraID, exp, gain, "guiding");
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
int save_img_guiding_local(char* fn){
    return save_img(&cam_info, fn, "guiding");
}
