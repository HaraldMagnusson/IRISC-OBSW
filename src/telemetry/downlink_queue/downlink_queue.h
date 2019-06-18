/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

typedef struct node downlink_node;

/* initialise the downlink queue component */
int init_downlink_queue(void);

/* queue up a message to be sent to ground 
   provided to external components */
int send_telemetry_local(int d, int p);

/* Return the data of the oldest message of the highest priority. */
int read_downlink_queue();
