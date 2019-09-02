/* -----------------------------------------------------------------------------
 * Component Name: GPS
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Store the current gps position of the gondola.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the orientation component */
int init_gps(void);

/* fetch the latest gps data */
void get_gps_local(gps_t* gps);

/* update the gps data */
void set_gps(gps_t* gps);

/* set the out of date flag on the gps data */
void gps_out_of_date(void);