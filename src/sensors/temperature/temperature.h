/* -----------------------------------------------------------------------------
 * Component Name: Temperature
 * Parent Component: Sensors
 * Author(s): 
 * Purpose: Store and protect the most recent temperature readings.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the temperature component */
int init_temperature(void* args);

/* return protected temperature measurements 
 * provided to external components
 */
void get_temperature_local( void );

/* update protected temperature measurements */
void set_temperature( void );
