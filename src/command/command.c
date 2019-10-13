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
#include "command.h"
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static void* thread_command(void* param);
static int handle_command(char command);

int init_command(void* args){

    return create_thread("command", thread_command, 35);
}

static void* thread_command(void* param){
    int ret;
    char command[1];

    while(1){
        if(read_elink(command, 1)==0){

            printf("Ostkaka: %d\n", command[0]);

            ret = handle_command(command[0]);
        }  
    }

    return SUCCESS;
}

static int handle_command(char command){

    char buffer[1400];

    printf("switch %d\n", command);

    switch(command){

        case CMD_REBOOT:
            break;

        case CMD_PING:
            send_telemetry_local("Pong", 1, 0, 0);
            break;

        case CMD_DATARATE:

            read_elink(buffer, 2);
            unsigned short datarate = *(unsigned short*)&buffer[0];
            set_datarate(datarate);

            snprintf(buffer, 1400, "Datarate set to: %d", datarate);

            send_telemetry_local(buffer, 1, 0, 0);

            break;

        default : /*  Default  */
            logging(ERROR, "downlink", "Unknown command");

    }

    return SUCCESS;
}
