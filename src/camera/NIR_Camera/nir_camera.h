/* -----------------------------------------------------------------------------
 * Component Name: NIR Camera
 * Parent Component: Camera
 * Author(s): 
 * Purpose: Provide an interface to the NIR camera to enable the capturing
 *          of images.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the nir camera component */
void init_nir_camera( void );

/* take a picture using the nir camera 
 * 
 * Might be better to have: start exposure & end exposure 
 * if camera api supports it.
 */
void nir_image( void );
