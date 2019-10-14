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

/* set offsets for the azimuth and altitude angle encoders */
void set_encoder_offsets_local(double az, double alt);

/* return the pid for the star tracker child process */
pid_t get_st_pid(void);

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_gain_l(int st_exp, int st_gain);

/* fetch a single sample from the encoder */
int enc_single_samp_l(encoder_t* enc);
