/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the downlink queue component */
int init_downlink_queue( void );

/* queue up a message to be sent to ground 
 * provided to external components
 */
int send_telemetry_local(int d, int p);

/* read the oldest message in the queue (FIFO) */
int read_downlink_queue( void );

int is_empty( void );