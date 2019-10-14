/* -----------------------------------------------------------------------------
 * Component Name: PID
 * Parent Component: Control System
 * Author(s): Adam Śmiałek
 * Purpose: Stabilise the telescope on the current target.
 *
 *
 * Functions for use as telecommands:
 *  - change_pid_values - changes current pid values until next mode change
 *  - change_mode_pid_values - permanently changes pid values for specified mode
 *
 * Functions for external call:
 *  - change_stabilization_mode - use to change to stabilization mode at the
 *                                start of exposure and back to tracking at the
 *                                end of it
 *  - pid_reset - to be used alongside change_stabilization_mode if deemed
 *                necessary
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

/* initialise the pid component */
int init_pid(void* args);

void pid_update(telescope_att_t* cur_att, motor_step_t* motor_out);

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
