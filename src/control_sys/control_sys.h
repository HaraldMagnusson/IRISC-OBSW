/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the control_sys component */
int init_control_sys(void* args);

/* return the current target to be tracked */
void get_target( void );

/* update the list of targets and priorities */
void set_target_list( void );

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang);
