/* -----------------------------------------------------------------------------
 * Component Name: Mode
 * Author(s): Harald Magnusson
 * Purpose: Store the current state of the software.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

enum mode_t{normal = '0', sleep = '1'};

/* initialise the mode component */
int init_mode( void );

/* get the current software state */
char get_mode( void );

/* set the current software state */
void set_mode( char mode );
