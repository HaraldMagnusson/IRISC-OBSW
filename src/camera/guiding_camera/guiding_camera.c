/* -----------------------------------------------------------------------------
 * Component Name: Guiding Camera
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface to the guiding camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "global_utils.h"
#include "camera_utils.h"
#include "guiding_camera.h"

ASI_CAMERA_INFO cam_info;

/* init_guiding_camera:
 * Set up and initialise the guiding camera.
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: failure to set up camera, log written to stderr
 */
int init_guiding_camera( void ){

    int ret = cam_setup(&cam_info, 'g');
    if(ret != SUCCESS){
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
 *      FAILURE: starting exposure failed, log written to stderr
 */
int expose_guiding_local(int exp, int gain){
    return expose(cam_info.CameraID, exp, gain);
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
int save_img_guiding_local(char* fn){
    return save_img(&cam_info, fn);
}

#if 0
    // printing available controls, not necessary for functionality
    int cont_num;
    ASIGetNumOfControls(id, &cont_num);
    ASI_CONTROL_CAPS cont[1];
    for(int i=0; i<cont_num; ++i){
        ASIGetControlCaps(id, i, cont);
        printf("%s\n", cont->Name);
        printf("\t%s\n\n", cont->Description);
        if(i == 9){
            //printf("%ld\n%ld\n%ld\n", cont->MaxValue, cont->MinValue, cont->DefaultValue);
        }
    }
#endif

#if 0
    // example of how to capture image

    int ret = expose_guide(50, 1);
    if(ret != SUCCESS){
        return FAILURE;
    }

    struct timespec polling_time = {0, EXP_STATUS_POLLING};

    while(save_img_guide() == EXP_NOT_READY){
        nanosleep(&polling_time, NULL);
    }

    if(ret == EXP_FAILED){
        return FAILURE;
    }

    return SUCCESS;
#endif
