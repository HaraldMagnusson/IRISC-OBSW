/* -----------------------------------------------------------------------------
 * Component Name: SPI
 * Author(s): 
 * Purpose: Provide initialisation and an interface for the communications
 *          over the SPI bus.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the spi component */
void init_spi( void );

/* listen for a message on the spi bus */
void read_spi( void );

/* send a message on the spi bus */
void write_spi( void );
