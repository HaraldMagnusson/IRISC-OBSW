/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): Adam Smialek, William Eriksson
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the telemetry component */
int init_telemetry(void);

/* put data into the downlink queue */
int send_telemetry(char *filepath, int p, int flag, unsigned short packets_sent);
