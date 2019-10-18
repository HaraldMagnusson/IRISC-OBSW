/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s): Niklas Ulfvarson, Harald Magnusson
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_star_tracker_poller(void* args);

/* return the pid for the star tracker child process */
pid_t get_st_pid_local(void);

/* set the exposure time (in microseconds) and gain for the star tracker */
void set_st_exp_ll(int st_exp);
void set_st_gain_ll(int st_gain);

int get_st_exp_ll(void);
