/* -----------------------------------------------------------------------------
 * Component Name: Command
 * Author(s): William Eriksson
 * Purpose: Accept and handle all commands coming from the ground station.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "global_utils.h"
#include "e_link.h"
#include "control_sys.h"
#include "downlink_queue.h"
#include "command.h"
#include "mode.h"
#include "sensors.h"

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
            { /* scope to avoid redeinition of target */
                read_elink(buffer, 2);
                double target = (double)*(short*)&buffer[0];

                move_az_to(target);

                snprintf(buffer, 1400, "Stepping AZ to: %lg", target);

                send_telemetry_local(buffer, 1, 0, 0);
            }
            break;

        case CMD_STP_ALT:
            { /* scope to avoid redeinition of target */
                read_elink(buffer, 2);
                double target = (double)*(short*)&buffer[0];

                move_alt_to(target);

                snprintf(buffer, 1400, "Stepping ALT to: %lg", target);

                send_telemetry_local(buffer, 1, 0, 0);
            }
            break;

        case CMD_ENC_OFFSETS:
            if(set_enc_offsets()){
                send_telemetry_local("Setting encoder offsets failed.", 1, 0, 0);
            }
            break;

        default : /*  Default  */
            logging(ERROR, "downlink", "Unknown command");

    }

    return SUCCESS;
}
