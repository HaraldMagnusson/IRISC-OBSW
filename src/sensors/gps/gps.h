/* -----------------------------------------------------------------------------
 * Component Name: GPS
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Store the current gps position of the gondola.
 * -----------------------------------------------------------------------------
 */

#pragma once

typedef struct{
    float lat;
    float lon;
    float alt;

    /* This flag shows if the available data is the latest (value: 0)
     * or if an error occured in the gps module while updating (value: 1).
     * If an error has occured, the data in (lat, lon, alt) is the
     * latest valid data.
     */
    char out_of_date;
} gps_t;

/* initialise the orientation component */
int init_gps(void);

/* fetch the latest gps data */
void get_gps(gps_t* gps);

/* update the gps data */
void set_gps(gps_t* gps);

/* set the out of date flag on the gps data */
void gps_out_of_date(void);