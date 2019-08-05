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
#include <fitsio.h>
#include <errno.h>

#include "global_utils.h"
#include "camera_utils.h"

int write_img(unsigned short* buffer, ASI_CAMERA_INFO* cam_info, char* fn);
void yflip(unsigned short* buffer, int width, int height);

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
            logging(ERROR, "INIT", "Incorrect camera name: %c", cam_name);
            return FAILURE;
    }

    /* seems like this one has to be called */
    ASIGetNumOfConnectedCameras();

    /* identify the correct camera */
    for(int ii=0; ii<2; ++ii){
        ret = ASIGetCameraProperty(cam_info, ii);

        if(ret == ASI_ERROR_INVALID_INDEX){
            return ENODEV;
        }

        if(cam_info->MaxHeight == height){
            break;
        }
    }
    int id = cam_info->CameraID;

    /* opening and initializating */
    ret = ASIOpenCamera(id);
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera", "Failed to open camera: %c. "
                "Return value: %d", cam_name, ret);
        return FAILURE;
    }

    ret = ASIInitCamera(id);
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera", "Failed to initialise camera: %c. "
                "Return value: %d", cam_name, ret);
        return FAILURE;
    }

    /* setting image format to 16 bit per pixel */
    ret = ASISetROIFormat(id, width, height, 1, ASI_IMG_RAW16);
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera", "Failed to set image format for camera: %c. "
                "Return value: %d", cam_name, ret);
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
 *      cam_name: name of camera for logging
 *
 * return:
 *      SUCCESS: operation is successful
 *      EREMOTEIO: starting exposure failed
 *      EIO: setting camera control values failed
 *      ENODEV: Camera not connected
 */
int expose(int id, int exp, int gain, char* cam_name){

    int ret;

    /* set exposure time in micro sec */
    ret = ASISetControlValue(id, ASI_EXPOSURE, exp, ASI_FALSE);
    if(ret == ASI_ERROR_INVALID_ID){
        logging(ERROR, "Camera",
                "Camera disconnected when starting exposure: %s", cam_name);
        return ENODEV;
    }
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera",
                "Failed to set exposure time for %s camera.", cam_name);
        return EIO;
    }

    /* set sensor gain */
    ret = ASISetControlValue(id, ASI_GAIN, gain, ASI_FALSE);
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera",
                "Failed to set sensor gain for %s camera.", cam_name);
        return EIO;
    }

    ASIStartExposure(id, ASI_FALSE);
    if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera",
                "Failed to start exposure of %s camera.", cam_name);
        return EREMOTEIO;
    }

    return SUCCESS;
}

/* save_img:
 * save_img will first check if exposure is still ongoing or has failed and
 * return if that is the case. Otherwise the image will be fetched from the
 * camera and saved.
 *
 * input:
 *      cam_info: info for relevant camera
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
 * TODO: System for file names and queueing up image for processing.
 */
int save_img(ASI_CAMERA_INFO* cam_info, char* fn, char* cam_name){

    int id = cam_info->CameraID, ret;

    /* check current exposure status */
    ASI_EXPOSURE_STATUS exp_stat;
    ASIGetExpStatus(id, &exp_stat);
    switch(exp_stat){
        case ASI_EXP_WORKING:
            return EXP_NOT_READY;
        case ASI_EXP_FAILED:
            return EXP_FAILED;
        case ASI_EXP_IDLE:
            logging(ERROR, "Camera", "save_img called before starting exposure");
            return EPERM;
        default:
            break;
    }

    /* buffer for bitmap */
    int width = cam_info->MaxWidth;
    int height = cam_info->MaxHeight;
    int buffer_size = width*height*2;
    unsigned char* buffer = (unsigned char*) malloc(buffer_size);

    if(buffer == NULL){
        logging(ERROR, "Camera", "Cannot allocate memory for image buffer");
        return ENOMEM;
    }

    /* fetch data */
    ret = ASIGetDataAfterExp(id, buffer, buffer_size);
    if(ret == ASI_ERROR_INVALID_ID){
        logging(ERROR, "Camera", "Camera disconnected when fetching data: %s", cam_name);
        return ENODEV;
    }
    else if(ret != ASI_SUCCESS){
        logging(ERROR, "Camera", "Failed to fetch data from %s camera.", cam_name);
        return EIO;
    }

    unsigned short* buff = (unsigned short*)buffer;

    for(int ii=0; ii<buffer_size/2; ++ii){
        buff[ii] = buff[ii]>>4;
    }

    yflip(buff, width, height);
    ret = write_img(buff, cam_info, fn);
    if(ret != SUCCESS){
        return ret;
    }

    free(buffer);
    return SUCCESS;
}

/* write_img:
 * Writes a bitmap to a .fit image file using fitsio
 *
 * input:
 *      buffer: a bitmap of size [height*width]
 *      cam_info: camera info object for camera capturing the image
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: write failed, fits error written to stderr
 */
int write_img(unsigned short* buffer, ASI_CAMERA_INFO* cam_info, char* fn){
    fitsfile* fptr;
    int ret = 0;

    long fpixel=1, naxis=2, nelements;
    long naxes[2] = {cam_info->MaxWidth, cam_info->MaxHeight};
    nelements = naxes[0] * naxes[1];

    int fn_len = strlen(fn);
    char fn_f[fn_len+2];
    fn_f[0] = '!';
    for(int ii=0; ii<fn_len+1; ++ii){
        fn_f[ii+1] = fn[ii];
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "saving image, filename: %s", fn_f);
        logging(DEBUG, "Camera", "creating file");
    #endif

    fits_create_file(&fptr, fn_f, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "creating img");
    #endif

    fits_create_img(fptr, SHORT_IMG, naxis, naxes, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "updating header");
    #endif
    long exposure, gain;
    ASI_BOOL nein = ASI_FALSE;
    ASIGetControlValue(cam_info->CameraID, ASI_EXPOSURE, &exposure, &nein);
    fits_update_key(fptr, TLONG, "EXPOINUS", &exposure,
            "Exposure time in us", &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    ASIGetControlValue(cam_info->CameraID, ASI_GAIN, &gain, &nein);
    fits_update_key(fptr, TLONG, "GAIN", &gain,
            "The ratio of output / input", &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing image");
    #endif
    fits_write_img(fptr, TSHORT, fpixel, nelements, buffer, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing data");
    #endif
    fits_write_date(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing checksum");
    #endif
    fits_write_chksum(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #ifdef CAMERA_DEBUG
        logging(DEBUG, "Camera", "closing file");
    #endif
    fits_close_file(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    return SUCCESS;
}

/* yflip:
 * vertically flips a bitmap
 *
 * input:
 *      width: pixel width of image
 *      height: pixel height of image
 *      buffer: a bitmap of size [height*width]
 */
void yflip(unsigned short* buffer, int width, int height){
    unsigned short (*buff)[width] = (unsigned short (*)[width])buffer;
    unsigned short tmp;

    for(int ii=0; ii<height/2; ++ii){
        for(int jj=0; jj<width; ++jj){
            tmp = buff[ii][jj];
            buff[ii][jj] = buff[height-1-ii][jj];
            buff[height-1-ii][jj] = tmp;
        }
    }
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
