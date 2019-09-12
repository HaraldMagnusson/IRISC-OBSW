/* -----------------------------------------------------------------------------
 * Component Name: E-Link
 * Author(s): William Eriksson
 * Purpose: Provide initialisation and an interface for the communications over
 *          the E-link to the ground station.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the elink component */
int init_elink( void );

/* provide downlink to ground */
int write_elink( char *buffer, int bytes);

/* Reads TC sent over elink*/
char* read_elink(int bytes);

void close_socket( void );

/* limit datarate for TM by setting time between each packet sent*/
int set_datarate (unsigned short datarate);
