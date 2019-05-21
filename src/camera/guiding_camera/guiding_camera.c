/* -----------------------------------------------------------------------------
 * Component Name: Guiding Camera
 * Parent Component: Camera
 * Author(s): 
 * Purpose: Provide an interface to the guiding camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>

#include "global_utils.h"
#include "ASICamera2.h"

int init_guiding_camera( void ){

    int num_cam = ASIGetNumOfConnectedCameras();
    printf("%d\n", num_cam);

    

    return SUCCESS;
}
