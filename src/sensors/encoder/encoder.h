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

/* initialise the orientation component */
int init_encoder(void);

/* fetch the latest encoder data */
void get_encoder_local(encoder_t* encoder);

/* update the encoder data */
void set_encoder(encoder_t* encoder);

/* set the out of date flag on the encoder data */
void encoder_out_of_date(void);
