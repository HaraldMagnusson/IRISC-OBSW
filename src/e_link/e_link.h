/* -----------------------------------------------------------------------------
 * Component Name: E-Link
 * Author(s): 
 * Purpose: Provide initialisation and an interface for the communications over
 *          the E-link to the ground station.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the elink component */
int init_elink(void* args);

/* provide uplink from ground */
void read_elink( void );

/* provide downlink to ground */
void write_elink( void );
