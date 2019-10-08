/* -----------------------------------------------------------------------------
 * Component Name: Controller
 * Parent Component: Control System
 * Author(s): Adam Śmiałek
 * Purpose: Stabilise the telescope on the current target.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

typedef struct{
    double kp, ki, kd;
} pid_values_t;

typedef struct{
    double az, alt;
} telescope_angles_t;

typedef struct{
    float time_in_seconds;
    double current_position,
           target_position,
           position_error,
           derivative,
           integral,
           pid_output;
} control_variables_t;

/* initialise the stabilization component */
int init_stabilization(void* args);
