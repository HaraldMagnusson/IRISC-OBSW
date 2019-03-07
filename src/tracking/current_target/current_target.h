/* -----------------------------------------------------------------------------
 * Component Name: Current Target
 * Parent Component: Tracking
 * Author(s): 
 * Purpose: Store the currently highest priority target.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the current target component */
int init_current_target( void );

/* update protected target to be tracked */
void set_target( void );

/* return protected target to be tracked */
void get_target( void );

/* update the list of targets and priorities */
void set_target_list( void );

/* return the list of targets and priorities */
void set_target_list( void );
