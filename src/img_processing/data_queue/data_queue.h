/* -----------------------------------------------------------------------------
 * Component Name: Data Queue
 * Parent Component: Img Processing
 * Author(s): 
 * Purpose: Hold images untill they can be compressed, stored and sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/**
 * Node structure declaration.
 */
typedef struct node {
    char filepath[100];     // Filepath of data to be compressed.
    int priority;       // Lower values indicate higher priority
    int type;           // Type of file
    struct node *next;  // Pointer to the node next on the list.

} data_node;

/* initialise the data queue component */
int init_data_queue(void* args);

/* Compress and store file, for type use IMAGE_MAIN or IMAGE_STARTRACKER */
int store_data_local(char *f, int p, int type);

/* Return the data of the oldest message of the highest priority. */
struct node read_data_queue();
