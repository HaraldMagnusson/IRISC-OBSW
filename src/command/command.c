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
#include "control_sys.h"
#include "command.h"
#include "mode.h"
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

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

            ret = handle_command(command[0]);
        }  
    }

    return SUCCESS;
}

static int handle_command(char command){

    char buffer[1400];
    short step;
    motor_step_t stp_st = {0, 0, 0};

    switch(command){

        case CMD_REBOOT:

            sync();
            reboot(RB_AUTOBOOT);
            break;

        case CMD_MODE:

            read_elink(buffer, 1);

            if(buffer[0]>=0 && buffer[0]<4){
                set_mode(buffer[0]);

                snprintf(buffer, 1400, "Mode set to: %c", buffer[0]);
                send_telemetry_local(buffer, 1, 0, 0);
            } else {
                snprintf(buffer, 1400, "Mode NOT set, unknown input: %c", buffer[0]);
                send_telemetry_local(buffer, 1, 0, 0);
            }

            break;

        case CMD_PING:
            send_telemetry_local("Pong", 1, 0, 0);
            break;

        case CMD_DATARATE:

            read_elink(buffer, 2);
            unsigned short datarate = *(unsigned short*)&buffer[0];
            if(datarate>0){
                set_datarate(datarate);
            }

            snprintf(buffer, 1400, "Datarate set to: %d", datarate);

            send_telemetry_local(buffer, 1, 0, 0);

            break;
        
        case CMD_STP_AZ:

            read_elink(buffer, 2);
            step = *(short*)&buffer[0];

            stp_st.az = step;

            step_az_alt(&stp_st);

            usleep(10000);

            stp_st.az = 0;

            step_az_alt(&stp_st);

            snprintf(buffer, 1400, "Stepping AZ: %d", step);

            send_telemetry_local(buffer, 1, 0, 0);

            break;

        case CMD_STP_ALT:

            read_elink(buffer, 2);
            step = *(short*)&buffer[0];

            stp_st.alt = step;

            step_az_alt(&stp_st);

            usleep(10000);

            stp_st.alt = 0;

            step_az_alt(&stp_st);

            snprintf(buffer, 1400, "Stepping ALT: %d", step);

            send_telemetry_local(buffer, 1, 0, 0);

            break;

        default : /*  Default  */
            logging(ERROR, "downlink", "Unknown command");

    }

    return SUCCESS;
}
