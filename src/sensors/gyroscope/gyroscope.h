/* -----------------------------------------------------------------------------
 * Component Name: Gyroscope
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keep track of the angular motion of the telescope.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_gyroscope(void* args);

void get_gyro_local(gyro_t* gyro);

void set_gyro(gyro_t* gyro);

/* set the out of date flag on the gyro data */
void gyro_out_of_date(void);
