/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): 
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#pragma once

// values for the PID controller
#define Kp  0
#define Ki  0
#define Kd  1

// threshold value for motor angular rate in deg/s
#define MOTOR_ANG_RATE_THRS 0.05

// values shared by the control system, have to be mutexed
double filter_current_position;    // taken from the star tracker / gyroscope filter
double tracking_output_angle;      // taken from the tracking algorithm
double stabilization_output_angle; // taken from the stabilization algorithm

/* initialise the control_sys component */
int init_control_sys(void);

/* return the current target to be tracked */
void get_target( void );

/* update the list of targets and priorities */
void set_target_list( void );
