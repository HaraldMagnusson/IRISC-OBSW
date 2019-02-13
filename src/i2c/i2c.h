/* -----------------------------------------------------------------------------
 * Component Name: I2C
 * Author(s): 
 * Purpose: Provide initialisation and an interface for the communications
 *          over the I2C bus.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the i2c component */
void init_i2c( void );

/* listen for a message on the i2c bus */
void read_i2c( void );

/* send a message on the i2c bus */
void write_i2c( void );
