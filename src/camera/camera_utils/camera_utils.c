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
            fprintf(stderr, "Incorrect camera name: %c\n", cam_name);
            return ENODEV;
    }

    /* seems like this one has to be called */
    ASIGetNumOfConnectedCameras();

    /* identify the correct camera */
    for(int ii=0; ii<2; ++ii){
        ret = ASIGetCameraProperty(cam_info, ii);

        if(ret == ASI_ERROR_INVALID_INDEX){
            fprintf(stderr, "Error: Camera not connected: %c\n", cam_name);
            return SUCCESS;
            // return ENODEV;
        }
        else if(ret != ASI_SUCCESS){
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
 *      FAILURE:       saving the image failed, log written to stderr
 *
 * TODO:
 *      1. fix system for filenames
 *      2. fix .fit header
 */
int save_img(ASI_CAMERA_INFO* cam_info, char* fn){

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
            fprintf(stderr, "save_img called before starting exposure\n");
            return FAILURE;
        default:
            break;
    }

    /* buffer for bitmap */
    int width = cam_info->MaxWidth;
    int height = cam_info->MaxHeight;
    int buffer_size = width*height*2;
    unsigned char* buffer = (unsigned char*) malloc(buffer_size);

    if(buffer == NULL){
        fprintf(stderr, "cannot allocate memory for image buffer\n");
        return FAILURE;
    }

    /* fetch data */
    ret = ASIGetDataAfterExp(id, buffer, buffer_size);
    if(ret != ASI_SUCCESS){
        fprintf(stderr, "failed to fetch data from camera: %s\n", cam_info->Name);
        return FAILURE;
    }

    unsigned short* buff = (unsigned short*)buffer;

    for(int ii=0; ii<buffer_size/2; ++ii){
        buff[ii] = buff[ii]>>4;
    }

    yflip(buff, width, height);
    printf("writing_img\n");
    ret = write_img(buff, cam_info, fn);
    if(ret != SUCCESS){
        return ret;
    }
    /*
        / TODO: system for filenames /
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
    */
    return SUCCESS;
}

/* write_img:
 * Writes a bitmap to a .fit image file using fitsio
 *
 * input:
 *      buffer: a bitmap of size [height*width]
 *      cam_info: camera info object for camera capturing the image
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

    printf("\n%s \n", fn);
    printf(" filename: %s\n\n", fn_f);

    printf("creating file\n");
    fits_create_file(&fptr, fn_f, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    printf("creating img\n");
    fits_create_img(fptr, SHORT_IMG, naxis, naxes, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    printf("updating header\n");
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

    printf("writing img\n");
    fits_write_img(fptr, TSHORT, fpixel, nelements, buffer, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    printf("writing date\n");
    fits_write_date(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    printf("writing checksum\n");
    fits_write_chksum(fptr, &ret);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    int data_ok, hdu_ok;
    fits_verify_chksum(fptr, &data_ok, &hdu_ok, &ret);
    printf("checksums check:\n"
            "\tdata: %d\n"
            "\thdu: %d\n",
            data_ok, hdu_ok);
    if(ret != 0){
        fits_report_error(stderr, ret);
        return FAILURE;
    }

    printf("closing file\n");
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
