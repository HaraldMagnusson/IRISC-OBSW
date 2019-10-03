/* -----------------------------------------------------------------------------
 * Component Name: Sanity Camera
 * Parent Component: Camera
 * Author(s): 
 * Purpose: Provide an interface to the sanity camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the sanity camera component */
int init_sanity_camera(void* args);

/* take a picture using the sanity camera 
 * provided to external components
 */
void sanity_image_local( void );
