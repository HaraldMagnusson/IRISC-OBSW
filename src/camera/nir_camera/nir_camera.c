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
#include <string.h>

#include "global_utils.h"
#include "camera_utils.h"
#include "img_processing.h"

static ASI_CAMERA_INFO cam_info;

static char out_fn[100], out_fp[100], tmp_fn[100];

/* init_nir_camera:
 * Set up and initialise the nir camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_nir_camera(void* args){

    strcpy(out_fp, get_top_dir());
    strcat(out_fp, "output/compression/");

    strcpy(tmp_fn, get_top_dir());
    strcat(tmp_fn, "output/compression/nir_tmp.fit");

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

static int img_cntr = 0;

/* save_img_nir:
 * save_img_nir will first check if exposure is still ongoing or has failed
 * and return if that is the case. Otherwise the image will be fetched from
 * the camera and saved.
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
int save_img_nir_local(void){


    int ret = save_img(&cam_info, tmp_fn, "NIR", NULL);
    if(ret){
        return ret;
    }

    /* make temporary file name for nir images */
    snprintf(out_fn, 100, "%snir%04d.fit", out_fp, img_cntr++);
    rename(tmp_fn, out_fn);

    queue_image(out_fn, IMAGE_MAIN);
    return SUCCESS;
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
int abort_exp_nir_local(void){
    snprintf(out_fn, 100, "%snir%04d.fit", out_fp, img_cntr++);

    int ret = abort_exp(&cam_info, out_fn, "NIR");
    if(ret){
        return ret;
    }

    queue_image(out_fn, IMAGE_MAIN);
    return SUCCESS;
}
