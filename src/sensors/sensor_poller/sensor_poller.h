/* -----------------------------------------------------------------------------
 * Component Name: Sensor Poller
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Poll sensors and update storage components.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the sensor poller component */
int init_sensor_poller(void* args);

/* return the pid for the star tracker child process */
pid_t get_st_pid(void);

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_gain_l(int exp, int st_gain);
