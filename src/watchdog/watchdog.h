/* -----------------------------------------------------------------------------
 * Component Name: Watchdog
 * Author(s): William Eriksson
 * Purpose: Initialise and regularly reset the watchdog.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the watchdog component */
int init_watchdog( void );

/* Stops thread_watchdog and disables the watchdog */
int stop_watchdog( void );
