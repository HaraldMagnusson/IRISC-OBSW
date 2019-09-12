/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): 
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the telemetry component */
int init_telemetry(void* args);

/* queue up a message to be sent to ground */
void send_telemetry( void );
