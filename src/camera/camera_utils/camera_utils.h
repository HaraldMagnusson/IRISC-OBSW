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
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: starting exposure failed, log written to stderr
 */
int expose(int id, int exp, int gain);

/* save_img
 * save_img will first check if exposure is still ongoing or has failed and 
 * return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 * 
 * input:
 *      cam_info for relevant camera
 *
 * return:
 *      SUCCESS:       operation is successful
 *      EXP_NOT_READY: exposure still ongoing, wait a bit and call again
 *      EXP_FAILED:    exposure failed and must be retried
 *      FAILURE:       saving the image failed, log written to stderr
 *
 * TODO:
 *      1. fix system for filenames
 *      2. fix .fit header
 */
int save_img(ASI_CAMERA_INFO* cam_info, char* fn);
