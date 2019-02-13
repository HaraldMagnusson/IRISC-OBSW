/* -----------------------------------------------------------------------------
 * Component Name: Guiding Camera
 * Parent Component: Camera
 * Author(s): 
 * Purpose: Provide an interface to the guiding camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the guiding camera component */
void init_guiding_camera( void );

/* take a picture using the guiding camera 
 * provided to external components
 */
void guiding_image_local( void );
