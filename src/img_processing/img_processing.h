/* -----------------------------------------------------------------------------
 * Component Name: Img Processing
 * Author(s): 
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#pragma once

#define IMAGE_MAIN 1
#define IMAGE_STARTRACKER 2

/* initialise the img processing component */
int init_img_processing(void* args);

/* enqueue an image with meta data in the queue to be processed. 
 *p is the priority. Type should be IMAGE_STARTRACKER or IMAGE_MAIN 
 */
void queue_image( const char *filepath, int type);

/* Give the next startracker image a higher priority */
void send_st(void);
