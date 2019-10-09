/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

extern pthread_mutex_t mutex_cond_cont_sys;
extern pthread_cond_t cond_cont_sys;

/* initialise the control_sys component */
int init_control_sys(void* args);

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang);
