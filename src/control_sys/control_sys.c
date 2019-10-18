/* -----------------------------------------------------------------------------
 * Component Name: Tracking
 * Author(s): Anja Möslinger, Adam Śmiałek, Harald Magnusson
 * Purpose: Stabilise the telescope. Keep track of the currently highest 
 *          priority target. Provide an interface to update target 
 *          priority list.
 * -----------------------------------------------------------------------------
 */

#include <string.h>
#include <pthread.h>

#include "global_utils.h"

#include "control_sys.h"

#include "stabilization.h"
#include "current_target.h"
#include "gimbal.h"
#include "target_selection.h"
#include "kalman_filter.h"
#include "pid.h"

#define MODULE_COUNT 6

/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
    {"tar__selection", &init_target_selection},
    {"current_target", &init_current_target},
    {"stabilization", &init_stabilization},
    {"kalman_filter", &init_kalman_filter},
    {"gimbal", &init_gimbal},
    {"pid", &init_pid}
};

int init_control_sys(void* args){

    /* init whatever in this module */
    return init_submodules(init_sequence, MODULE_COUNT);
}

/* Set the error thresholds for when to start exposing camera */
void set_error_thresholds(double az, double alt_ang){
    set_error_thresholds_local(az, alt_ang);
}

int step_az_alt(motor_step_t* steps){

    return step_az_alt_local(steps);
}

int step_roll(motor_step_t* steps){

    return step_roll_local(steps);
}
/* Rotate the telescope to center of horizontal field of view, 45 deg up */
void center_telescope(void){
    center_telescope_l();
}

/* Rotate the telescope to a target in az relative to gondola,
 * with an accuracy of 1 degree
 */
void move_az_to(double target){
    move_az_to_l(target);
}

/* Rotate the telescope to a target in alt relative to gondola,
 * with an accuracy of 1 degree
 */
void move_alt_to(double target){
    move_alt_to_l(target);
}

/* resets the field rotator position to clockwise end */
void reset_field_rotator(void){
    reset_field_rotator_l();
}
