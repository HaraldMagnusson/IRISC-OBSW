/* -----------------------------------------------------------------------------
 * Component Name: Data Queue
 * Parent Component: Img Processing
 * Author(s): 
 * Purpose: Hold images along with housekeeping data until handler is ready.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the data queue component */
int init_data_queue( void );

/* enqueue an image with meta data in the queue to be processed 
 * provided to external components
 */
void queue_image_local( void );

/* read the oldest message in the queue (FIFO) */
void read_queue( void );
