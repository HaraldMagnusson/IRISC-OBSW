/* -----------------------------------------------------------------------------
 * Component Name: Gimbal
 * Parent Component: Tracking
 * Author(s): 
 * Purpose: Provide an interface for the control of the gimbal motors.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

/* initialise the gimbal component */
int init_gimbal(void* args);

int step_gimbal(int az_stepps, int el_stepps, int rt_stepps);
