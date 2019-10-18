/* -----------------------------------------------------------------------------
 * Component Name: Gimbal
 * Parent Component: Control System
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface for the control of the gimbal motors.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

#include "control_sys.h"

/* initialise the gimbal component */
int init_gimbal(void* args);

int step_az_alt_local(motor_step_t* steps);

int step_roll_local(motor_step_t* steps);

/* Rotate the telescope to center of horizontal field of view, 45 deg up */
void center_telescope_l(void);

/* Rotate the telescope to a target in az relative to gondola,
 * with an accuracy of 1 degree
 */
void move_az_to_l(double target);

/* Rotate the telescope to a target in alt relative to gondola,
 * with an accuracy of 1 degree
 */
void move_alt_to_l(double target);

/* resets the field rotator position to clockwise end */
void reset_field_rotator_l(void);
