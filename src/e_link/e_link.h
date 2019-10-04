/* -----------------------------------------------------------------------------
 * Component Name: E-Link
 * Author(s): William Eriksson
 * Purpose: Provide initialisation and an interface for the communications over
 *          the E-link to the ground station.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the elink component */
int init_elink(void* args);

/* provide downlink to ground */
int write_elink( char *buffer, int bytes);

/* Reads TC sent over elink*/
int read_elink(char *buffer, int bytes);

void close_socket( void );

/* limit datarate for TM by setting time between each packet sent*/
int set_datarate (unsigned short datarate);
