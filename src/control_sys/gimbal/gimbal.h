/* -----------------------------------------------------------------------------
 * Component Name: Gimbal
 * Parent Component: Control System
 * Author(s): William Eriksson
 * Purpose: Provide an interface for the control of the gimbal motors.
 *
 * -----------------------------------------------------------------------------
 */

#pragma once

typedef struct{
    int az, alt, roll;
} motor_step_t;

/* initialise the gimbal component */
int init_gimbal(void* args);

int step_az_alt(motor_step_t* steps);

int step_roll(motor_step_t* steps);

/* Rotate the telescope to center of horizontal field of view, 45 deg up */
void center_telescope_l(void);
