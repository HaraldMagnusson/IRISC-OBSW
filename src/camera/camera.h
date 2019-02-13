/* -----------------------------------------------------------------------------
 * Component Name: Camera
 * Author(s): 
 * Purpose: Selecting targets, camera settings, capturing images, and providing
 *          an interface to each camera.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the camera component */
void init_camera( void );

/* take a picture using the sanity camera */
void sanity_image( void );

/* take a picture using the guiding camera */
void guiding_image( void );
