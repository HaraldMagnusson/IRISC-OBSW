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

static struct timespec start_time[2];
static char exp_start_datetime[2][20];
static int timeref[2];

static int write_img(unsigned short* buffer, ASI_CAMERA_INFO* cam_info, char* fn, struct timespec* exp_time);
static void yflip(unsigned short* buffer, int width, int height);

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
 *      cam_name: name of camera for logging
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

    ffgstm(exp_start_datetime[id], &timeref[id], &ret);
    clock_gettime(CLOCK_REALTIME, &start_time[id]);
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
int save_img(ASI_CAMERA_INFO* cam_info, char* fn, char* cam_name, struct timespec* exp_time){

    int id = cam_info->CameraID, ret;

    /* check current exposure status */
    ASI_EXPOSURE_STATUS exp_stat;
    ASIGetExpStatus(id, &exp_stat);
    switch(exp_stat){
        case ASI_EXP_WORKING:
            return EXP_NOT_READY;
        case ASI_EXP_FAILED:
            logging(ERROR, "Camera", "Exposure of %s camera failed", cam_name);
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

    ret = write_img(buff, cam_info, fn, exp_time);

    free(buffer);
    return ret;
}

/* write_img:
 * Writes a bitmap to a .fit image file using fitsio
 *
 * input:
 *      buffer: a bitmap of size [height*width]
 *      cam_info: camera info object for camera capturing the image
 *      fn: filename to save image as
 *      exp_time: calculated exposure time if the exposure was aborted
 *                NULL if not
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: write failed, fits error written to stderr
 */
static int write_img(unsigned short* buffer, ASI_CAMERA_INFO* cam_info, char* fn, struct timespec* exp_time){
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

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "saving image, filename: %s", fn_f);
        logging(DEBUG, "Camera", "creating file");
    #endif

    fits_create_file(&fptr, fn_f, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "creating img");
    #endif

    fits_create_img(fptr, SHORT_IMG, naxis, naxes, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "updating header");
    #endif
    long exposure, gain;
    ASI_BOOL pb_auto = ASI_FALSE;
    if(exp_time != NULL){
        exposure = exp_time->tv_sec * 1000000 + exp_time->tv_nsec / 1000;
        #if CAMERA_DEBUG
            logging(DEBUG, "Camera", "aborted after exposing for %ld microseconds", exposure);
        #endif
    }
    else{
        ASIGetControlValue(cam_info->CameraID, ASI_EXPOSURE, &exposure, &pb_auto);
    }
    fits_update_key(fptr, TLONG, "EXPOINUS", &exposure,
            "Exposure time in us", &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    ASIGetControlValue(cam_info->CameraID, ASI_GAIN, &gain, &pb_auto);
    fits_update_key(fptr, TLONG, "GAIN", &gain,
            "The ratio of output / input", &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing image");
    #endif
    fits_write_img(fptr, TSHORT, fpixel, nelements, buffer, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing data");
    #endif
    fits_update_key(fptr, TSTRING, "DATE", exp_start_datetime[cam_info->CameraID],
            "Exposure start time (YYYY-MM-DDThh:mm:ss UTC)", &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
        logging(DEBUG, "Camera", "writing checksum");
    #endif
    fits_write_chksum(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    #if CAMERA_DEBUG
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
static void yflip(unsigned short* buffer, int width, int height){
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
int abort_exp(ASI_CAMERA_INFO* cam_info, char* fn, char* cam_name){

    struct timespec stop_time, exp_time;
    logging(WARN, "Camera", "Aborting exposure of %s camera", cam_name);

    clock_gettime(CLOCK_REALTIME, &stop_time);
    int ret = ASIStopExposure(cam_info->CameraID);
    if(ret == ASI_ERROR_INVALID_ID){
        logging(ERROR, "Camera",
            "Camera disconnected when aborting exposure: %s", cam_name);
    }
    else if(ret != SUCCESS){
        logging(ERROR, "Camera", "Failed to abort exposure of %s camera", cam_name);
    }

    if(stop_time.tv_nsec < start_time[cam_info->CameraID].tv_nsec){
        exp_time.tv_sec = stop_time.tv_sec - start_time[cam_info->CameraID].tv_sec - 1;
        exp_time.tv_nsec = stop_time.tv_nsec + 1000000000 - start_time[cam_info->CameraID].tv_nsec;
    }
    else{
        exp_time.tv_sec = stop_time.tv_sec - start_time[cam_info->CameraID].tv_sec;
        exp_time.tv_nsec = stop_time.tv_nsec - start_time[cam_info->CameraID].tv_nsec;
    }

    return save_img(cam_info, fn, cam_name, &exp_time);
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
