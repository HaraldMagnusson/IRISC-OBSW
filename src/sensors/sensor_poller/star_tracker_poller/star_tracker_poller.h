/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s):
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */

#pragma once

#include <sys/types.h>

int init_star_tracker_poller(void* args);

pid_t get_star_tracker_pid(void);
