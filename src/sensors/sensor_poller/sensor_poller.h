/* -----------------------------------------------------------------------------
 * Component Name: Sensor Poller
 * Parent Component: Sensors
 * Author(s): 
 * Purpose: Poll sensors and update storage components.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the sensor poller component */
int init_sensor_poller( void );

/* set offsets for the azimuth and altitude angle encoders */
void set_encoder_offsets_local(double az, double alt);
