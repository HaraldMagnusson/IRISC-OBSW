/* -----------------------------------------------------------------------------
 * Component Name: Sensors
 * Author(s): 
 * Purpose: Poll sensors and store the latest data. Provide an interface for
 *          other components to read the data.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the sensors component */
int init_sensors( void );

/* return temperature measurements */
void get_temperature( void );

/* return position and orientation measurements */
void get_orientation( void );

/* return current position of the sun */
void get_sun_pos( void );
