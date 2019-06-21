/* -----------------------------------------------------------------------------
 * Component Name: Camera Utils
 * Parent Component: Camera
 * Author(s): Harald Magnusson
 * Purpose: Provide utilies for the use of ZWO ASI cameras.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "global_utils.h"
#include "camera_utils.h"

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
int cam_setup(ASI_CAMERA_INFO* cam_info, char cam_name){

    int ret, height, width;

    switch(cam_name){
        case 'n':
            height = NIR_HEIGHT;
            width = NIR_WIDTH;
            break;
        case 'g':
            height = GUIDE_HEIGHT;
            width = GUIDE_WIDTH;
            break;
        default:
            fprintf(stderr, "Incorrect camera name.\n");
            return FAILURE;
    }

    /* seems like this one has to be called */
    ASIGetNumOfConnectedCameras();

    /* identify the correct camera */
    for(int ii=0; ii<2; ++ii){
        ret = ASIGetCameraProperty(cam_info, ii);
        
        if(ret != ASI_SUCCESS){
            fprintf(stderr, "Failed to fetch camera properties "
                    "(ASIGetCameraProperty), return value: %d\n", ret);
            return FAILURE;
        }

        if(cam_info->MaxHeight == height){
            break;
        }
    }
    int id = cam_info->CameraID;

    /* opening and initializating */
    ret = ASIOpenCamera(id);
    if(ret != ASI_SUCCESS){ 
        fprintf(stderr, "Failed to open the guiding camera "
                "(ASIOpenCamera), return value: %d\n", ret);

        return FAILURE;
    }

    ret = ASIInitCamera(id);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "Failed to initialise the guiding camera "
                "(ASIInitCamera), return value: %d\n", ret);

        return FAILURE;
    }

    /* setting image format to 16 bit per pixel */
    ret = ASISetROIFormat(id, width, height, 1, 2);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "Failed to set ROI format for the guiding camera "
            "(ASISetROIFormat), return value: %d\n", ret);

        return FAILURE;
    }

    return ASI_SUCCESS;
}

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
int expose(int id, int exp, int gain){

    int ret;

    // set exposure time in micro sec
    ret = ASISetControlValue(id, ASI_EXPOSURE, exp, ASI_FALSE);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "Failed to set exposure time "
                "(ASISetControlValue), return value: %d\n", ret);
        return FAILURE;
    }

    // set sensor gain
    ret = ASISetControlValue(id, ASI_GAIN, gain, ASI_FALSE);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "Failed to set sensor gain "
                "(ASISetControlValue), return value: %d\n", ret);
        return FAILURE;
    }

    ASIStartExposure(id, 0);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "Failed to start exposure "
                "(ASIStartExposure), return value: %d\n", ret);
        return FAILURE;
    }

    return SUCCESS;
}

/* save_img:
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
 *      FAILURE:       storing the image failed, log written to stderr
 *
 * TODO:
 *      1. fix system for filenames
 *      2. fix .fit header
 */
int save_img(ASI_CAMERA_INFO* cam_info){

    int id = cam_info->CameraID;

    /* check current exposure status */
    ASI_EXPOSURE_STATUS exp_stat;
    ASIGetExpStatus(id, &exp_stat);
    switch(exp_stat){
        case EXP_NOT_READY:
            return EXP_NOT_READY;
        case EXP_FAILED:
            return EXP_FAILED;
        default:
            break;
    }

    /* buffer for bitmap */
    int width = cam_info->MaxWidth;
    int height = cam_info->MaxHeight;

    long buffer_size = width*height*2;
    unsigned char* buffer = (unsigned char*) malloc(buffer_size);

    /* fetch data */
    ASIGetDataAfterExp(id, buffer, buffer_size);

    /* TODO: system for filenames */
    FILE* fd;
    char* fn = "/tmp/hax.fit";

    fd = fopen(fn, "w");
    if(fd == NULL){
        fprintf(stderr, "Failed to open file to save guiding camera image: "
            "%s\n", strerror(errno));
        return FAILURE;
    }

    int ret = fwrite(buffer, 1, buffer_size, fd);
    if(ret != buffer_size){
        fprintf(stderr, "Failed to write guiding camera image to file: "
            "%s\n", strerror(errno));
        return FAILURE;
    }
    free(buffer);

    ret = fclose(fd);
    if(ret != 0){
        fprintf(stderr, "Failed to close guiding camera image file: "
            "%s\n", strerror(errno));
        return FAILURE;
    }

    printf("written to file\n");

    return SUCCESS;
}
