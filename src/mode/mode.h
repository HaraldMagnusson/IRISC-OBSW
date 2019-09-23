/* -----------------------------------------------------------------------------
 * Component Name: Mode
 * Author(s): Harald Magnusson
 * Purpose: Store the current state of the software.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

#define NORMAL  0
#define SLEEP   1
#define RESET   2
#define WAKE_UP 3

/* initialise the mode component */
int init_mode(void* args);

/* get the current software state */
char get_mode(void);

/* set the current software state */
void set_mode(char mode);
