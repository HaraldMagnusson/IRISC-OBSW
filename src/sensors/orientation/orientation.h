/* -----------------------------------------------------------------------------
 * Component Name: Orientation
 * Parent Component: Sensors
 * Author(s): 
 * Purpose: Store the current orientation and position of the gondola and
 *          telescope.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the orientation component */
void init_orientation( void );

/* return protected position and orientation measurements 
 * provided to external components
 */
void get_orientation_local( void );

/* update protected position and orientation measurements */
void set_orientation( void );
