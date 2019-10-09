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
    double time_in_seconds,
           current_position,
           target_position,
           position_error,
           derivative,
           integral,
           pid_output;
} control_variables_t;

/* initialise the stabilization component */
int init_stabilization(void* args);


/* Changes pid parameters until next mode change
 *
 * First argument has to be motor id, that is:
 *  1 for azimuth control
 *  2 for altitude angle control.
 *
 * The three other arguments are kp, ki, kd in double type.
 */
int change_pid_values(int motor_id, double new_p, double new_i, double new_d);

/* Changes chosen mode pid parameters forever
 *
 * First argument has to be motor id, that is:
 *  1 for azimuth control
 *  2 for altitude angle control.
 *
 * Second argument has to be stabilization mode, that is:
 *  1 for target acquisition
 *  2 for stabilization.
 */
int change_mode_pid_values(int motor_id, int mode_id, double new_p, double new_i, double new_d);

/* Resets the value for integral part and for the position error */
void pid_reset();

/* Sets pid parameters to and from stabilization mode
 *
 * 1 - on (ready for stabilization)
 * 0 - off (ready for tracking)
 */
int change_stabilization_mode(int on_off);
