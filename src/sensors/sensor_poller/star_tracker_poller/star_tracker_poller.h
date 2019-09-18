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
