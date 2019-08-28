/* -----------------------------------------------------------------------------
 * Component Name: Gyroscope
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keep track of the angular motion of the telescope.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

typedef struct{
    double x, y, z;

    /* This flag shows if the available data is the latest (value: 0)
     * or if an error occured in the gyro module while updating (value: 1).
     * If an error has occured, the data in (lat, lon, alt) is the
     * latest valid data.
     */
    char out_of_date;
} gyro_t;

int init_gyroscope(void);

void get_gyro(gyro_t* gyro);

void set_gyro(gyro_t* gyro);

/* set the out of date flag on the gyro data */
void gyro_out_of_date(void);
