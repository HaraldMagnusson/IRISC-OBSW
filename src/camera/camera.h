/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): Harald Magnusson
 * Purpose: Selecting targets, camera settings, capturing images, and providing
 *          an interface to each camera.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the camera component */
int init_camera( void );

/* take a picture using the sanity camera */
void sanity_image( void );

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
int expose_guiding(int exp, int gain);

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
int save_img_guiding(void);

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
int expose_nir(int exp, int gain);

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
int save_img_nir(void);
