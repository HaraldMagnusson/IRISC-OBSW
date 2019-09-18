/* -----------------------------------------------------------------------------
 * Component Name: Image Handler
 * Parent Component: Img Processing
 * Author(s): 
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the image handler component */
int init_image_handler( void );

/* Compress and store a file */
int compression_stream(const char* in_filename, const char* out_filename);
