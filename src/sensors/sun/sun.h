/* -----------------------------------------------------------------------------
 * Component Name: Sun
 * Parent Component: Sensors
 * Author(s): 
 * Purpose: Keep track of the current position of the sun.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the sun component */
int init_sun( void );

/* return the current sun position 
 * provided to external components
 */
void get_sun_pos_local( void );
