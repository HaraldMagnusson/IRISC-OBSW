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
