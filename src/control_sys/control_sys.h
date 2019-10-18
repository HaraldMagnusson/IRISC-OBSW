/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): Anja Möslinger, Adam Śmiałek, Harald Magnusson
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

#include <pthread.h>

extern pthread_mutex_t mutex_cond_cont_sys;
extern pthread_cond_t cond_cont_sys;

extern pthread_mutex_t mutex_cond_sel_track;
extern pthread_cond_t cond_sel_track;

typedef struct{
    int az, alt, roll;
} motor_step_t;

/* initialise the control_sys component */
int init_control_sys(void* args);

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang);

int step_az_alt(motor_step_t* steps);

int step_roll(motor_step_t* steps);

/* Rotate the telescope to center of horizontal field of view, 45 deg up */
void center_telescope(void);

/* Rotate the telescope to a target in az relative to gondola,
 * with an accuracy of 1 degree
 */
void move_az_to(double target);

/* Rotate the telescope to a target in alt relative to gondola,
 * with an accuracy of 1 degree
 */
void move_alt_to(double target);

/* resets the field rotator position to clockwise end */
void reset_field_rotator(void);

void set_error_thresholds_az(double az);
void set_error_thresholds_alt(double alt_ang);

void set_nir_exp(int exp);
void set_nir_gain(int gain);
