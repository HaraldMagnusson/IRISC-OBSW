/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek, William Eriksson
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/**
 * Node structure declaration.
 */
typedef struct node {
    char *filepath;           // Filepath of data to be send as a telemetry.
    int priority;       // Lower values indicate higher priority
    int flag;           // If 1 data is in a file, if 0 data as a string.
    unsigned short packets_sent;
    struct node *next;  // Pointer to the node next on the list.

} downlink_node;

/* initialise the downlink queue component */
int init_downlink_queue(void);

/* queue up a message to be sent to ground 
   provided to external components. If flag is 1 f should be a filepath, if 0 f is a string */
int send_telemetry_local(char *f, int p, int flag, unsigned short packets_sent);

/* Return the data of the oldest message of the highest priority. */
struct node read_downlink_queue();

/* Return the highest priority in the queue */
int queue_priority();
