/* -----------------------------------------------------------------------------
 * Component Name: Encoder
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keeps track of the telescope attitude relative to the gondola.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

int init_encoder( void );

typedef struct{
    double ra_motor;
    double dec_motor;
    double ra_gimbal;
    double dec_gimbal;

    /* This flag shows if the available data is the latest (value: 0)
     * or if an error occured in the gps module while updating (value: 1).
     * If an error has occured, the data in the struct is the
     * latest valid data.
     */
    char out_of_date;
} encoder_t;

/* initialise the orientation component */
int init_encoder(void);

/* fetch the latest encoder data */
void get_encoder(encoder_t* encoder);

/* update the encoder data */
void set_encoder(encoder_t* encoder);

/* set the out of date flag on the encoder data */
void encoder_out_of_date(void);
