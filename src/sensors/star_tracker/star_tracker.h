/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keep track of the absolute attitude of the telescope.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_star_tracker(void* args);

/* fetch the latest star tracker data */
void get_star_tracker_local(star_tracker_t* st);

/* update the star trackar data */
void set_star_tracker(star_tracker_t* st);

/* set the out of date flag on the star trackar data */
void st_out_of_date(void);
