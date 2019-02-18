/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the tracking component */
int init_tracking( void );

/* return the current target to be tracked */
void get_target( void );

/* update the list of targets and priorities */
void set_target_list( void );
