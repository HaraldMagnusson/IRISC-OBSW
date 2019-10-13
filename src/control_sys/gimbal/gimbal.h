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
