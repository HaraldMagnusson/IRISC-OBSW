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
int init_img_processing( void );

/* enqueue an image with meta data in the queue to be processed */
void queue_image( const char *filepath, int p , int type);
