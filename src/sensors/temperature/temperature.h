/* -----------------------------------------------------------------------------
 * Component Name: Temperature
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
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
void get_temp_l(temp_t* temp);

/* update protected temperature measurements */
void set_temp(temp_t* temp);

/* set the out of date flag on the temp data */
void temp_out_of_date(void);
