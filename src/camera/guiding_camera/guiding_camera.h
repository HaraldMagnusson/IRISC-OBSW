/* -----------------------------------------------------------------------------
 * Component Name: Guiding Camera
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface to the guiding camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* init_guiding_camera:
 * Set up and initialise the guiding camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_guiding_camera(void);

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
int expose_guiding_local(int exp, int gain);

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
int save_img_guiding_local(void);
