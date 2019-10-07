/* -----------------------------------------------------------------------------
 * Component Name: Camera Utils
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide utilies for the use of ZWO ASI cameras.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

#include "ASICamera2.h"

/* camera resolutions */
#define NIR_WIDTH 5496
#define NIR_HEIGHT 3672
#define GUIDE_WIDTH 1936
#define GUIDE_HEIGHT 1096

/* cam_setup:
 * Set up and initialize a given ZWO ASI camera.
 *
 * input:
 *      cam_name specifies which camera to initialize:
 *          'n' for nir camera
 *          'g' for guiding camera
 *
 * output:
 *      cam_info: info object for relevant camera
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 *      ENODEV: incorrect camera name or camera disconnected
 */
int cam_setup(ASI_CAMERA_INFO* cam_info, char cam_name);

/* expose:
 * Start an exposure of a ZWO ASI camera. Call save_img to store store
 * image after exposure
 *
 * input:
 *      id: camera id found in ASI_CAMERA_INFO
 *      exp: the exposure time in microseconds
 *      gain: the sensor gain
 *      cam_name: name of camera for logging
 *
 * return:
 *      SUCCESS: operation is successful
 *      EREMOTEIO: starting exposure failed
 *      EIO: setting camera control values failed
 *      ENODEV: Camera not connected
 */
int expose(int id, int exp, int gain, char* cam_name);

/* save_img:
 * save_img will first check if exposure is still ongoing or has failed and
 * return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 *
 * input:
 *      cam_info: info for relevant camera
 *      fn: filename to save image as
 *      cam_name: name of camera for logging
 *      exp_time: calculated exposure time if the exposure was aborted,
 *                NULL if not
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
 * TODO: System for file names and queueing up image for processing.
 */
int save_img(ASI_CAMERA_INFO* cam_info, char* fn, char* cam_name, struct timespec* exp_time);

/* abort_exp:
 * Abort an ongoing exposure and save the image.
 *
 * input:
 *      cam_info: info for relevant camera
 *      fn: filename to save image as
 *      cam_name: name of camera for logging
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
int abort_exp(ASI_CAMERA_INFO* cam_info, char* fn, char* cam_name);
