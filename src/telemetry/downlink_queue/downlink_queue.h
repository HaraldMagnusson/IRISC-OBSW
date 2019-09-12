/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): 
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the downlink queue component */
int init_downlink_queue(void* args);

/* queue up a message to be sent to ground 
 * provided to external components
 */
void send_telemetry_local( void );

/* read the oldest message in the queue (FIFO) */
void read_queue( void );
