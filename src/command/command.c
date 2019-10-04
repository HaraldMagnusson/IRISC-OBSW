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
#include <string.h>

#define COMMAND_REBOOT 10
#define COMMAND_UPDATE_TARGET 11
#define COMMAND_CALIBRATE_TRACKING 12
#define COMMAND_DOWNLINK_DATARATE 13
#define COMMAND_CAMERA_SETTINGS 14
#define COMMAND_CAMERA_CAPTURE 15

static void* thread_command(void* param);
static int handle_command(unsigned short command);

int init_command(void* args){

    return create_thread("command", thread_command, 35);
}

static void* thread_command(void* param){
    char buffer[1400];
    int ret;
    unsigned short command;

    while(1){
        read_elink(buffer, 2);
        command = *(unsigned short*)&buffer[0];

        ret = handle_command(command);  
    }

    return SUCCESS;
}

static int handle_command(unsigned short command){

    char buffer[1400];

    switch(command){

        case COMMAND_REBOOT:
            break;

        case COMMAND_UPDATE_TARGET:
            break;

        case COMMAND_CALIBRATE_TRACKING:
            break;

        case COMMAND_DOWNLINK_DATARATE:

            read_elink(buffer, 2);
            unsigned short datarate = *(unsigned short*)&buffer[0];
            set_datarate(datarate);
            strncpy(buffer, "Datarate set to: ", 1400);

            char* temp_char = (char*)&datarate;
            buffer[17] = temp_char[0];
            buffer[18] = temp_char[1];
            buffer[19] = '\0';

            send_telemetry_local(buffer, 1, 0, 0);

            break;

        case COMMAND_CAMERA_SETTINGS:
            break;

        case COMMAND_CAMERA_CAPTURE:
            break;

        default : /*  Default  */
            logging(ERROR, "downlink", "Unknown command");

    }

    return SUCCESS;
}
