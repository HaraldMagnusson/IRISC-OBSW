/* -----------------------------------------------------------------------------
 * Component Name: Controller
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

#include "global_utils.h"

int init_stabilization(void* args){
    return SUCCESS;
}

#if 0
// structure for control system thread with KF + PID
pthread_mutex_t mutex_cond_cont_sys = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cont_sys = PTHREAD_COND_INITIALIZER;

static void* control_sys_thread(void* args){
    pthread_mutex_lock(&mutex_cond_cont_sys);

    struct timespec wake_time;

    double motor_out[2];
    telescope_att_t cur_pos;

    while(1){

        pthread_cond_wait(&cond_cont_sys, &mutex_cond_cont_sys);

        clock_gettime(CLOCK_MONOTONIC, &wake_time);

        while(get_mode() != RESET){

            kf_update(&cur_pos);
            pid_update(&cur_pos, motor_out);

            motor_output(motor_out);

            wake_time.tv_nsec += CONTROL_SYS_WAIT;
            if(wake_time.tv_nsec >= 1000000000){
                wake_time.tv_sec++;
                wake_time.tv_nsec -= 1000000000;
            }
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
        }
    }

    return NULL;
}
#endif
