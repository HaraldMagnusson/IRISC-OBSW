/* -----------------------------------------------------------------------------
 * Component Name: Command
 * Author(s): William Eriksson
 * Purpose: Accept and handle all commands coming from the ground station.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include "e_link.h"
#include "downlink_queue.h"
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>

#define COMMAND_REBOOT 10
#define COMMAND_UPDATE_TARGET 11
#define COMMAND_CALIBRATE_TRACKING 12
#define COMMAND_DOWNLINK_DATARATE 13
#define COMMAND_CAMERA_SETTINGS 14
#define COMMAND_CAMERA_CAPTURE 15

static pthread_t thread_command_t;
static pthread_attr_t thread_attr;
static struct sched_param param;
static int ret;

static void* thread_command(void* param);
static int handle_command(unsigned short command);

int init_command( void ){

ret = pthread_attr_init(&thread_attr);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_init for e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setstacksize of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    param.sched_priority = 50;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_create(&thread_command_t, &thread_attr, thread_command, NULL);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_create of socket component. "
            "Return value: %d\n", ret);
        return ret;
    }

    return SUCCESS;
}

static void* thread_command(void* param){
    char *buffer;
    int ret;
    unsigned short command;

    while(1){
        buffer = read_elink(2);
        command = *(unsigned short*)&buffer[0];

        ret = handle_command(command);  
    }

    return SUCCESS;
}

static int handle_command(unsigned short command){

    char *buffer = malloc(1394);

    switch(command){

        case COMMAND_REBOOT:
            break;

        case COMMAND_UPDATE_TARGET:
            break;

        case COMMAND_CALIBRATE_TRACKING:
            break;

        case COMMAND_DOWNLINK_DATARATE:

            buffer = read_elink(2);
            unsigned short datarate = *(unsigned short*)&buffer[0];
            set_datarate(datarate);
            buffer = "Datarate set to: ";

            char* temp_char = (char*)&datarate;
            buffer[17] = temp_char[0];
            buffer[18] = temp_char[1];
            buffer[19] = '0';

            send_telemetry_local(buffer, 1, 0, 0);

            break;

        case COMMAND_CAMERA_SETTINGS:
            break;

        case COMMAND_CAMERA_CAPTURE:
            break;

        default : /*  Default  */
            printf("Error command\n");

    }

    return SUCCESS;
}