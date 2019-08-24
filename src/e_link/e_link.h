/* -----------------------------------------------------------------------------
 * Component Name: E-Link
 * Author(s): 
 * Purpose: Provide initialisation and an interface for the communications over
 *          the E-link to the ground station.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the elink component */
int init_elink( void );

/* provide downlink to ground */
int write_elink( char *buffer, int bytes);

void close_socket( void );
