/* -----------------------------------------------------------------------------
 * Component Name: Stabilization
 * Parent Component: Control System
 * Author(s): Harald Magnusson
 * Purpose: Combining the kalman filter, pid controller, and motor control
 *          to create the entire control system.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "current_target.h"
#include "global_utils.h"
#include "mode.h"
#include "control_sys.h"
#include "gimbal.h"
#include "pid.h"
#include "kalman_filter.h"

static void* control_sys_thread(void* args);

int init_stabilization(void* args){
    return create_thread("control_system", control_sys_thread, 30);
}

pthread_mutex_t mutex_cond_cont_sys = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cont_sys = PTHREAD_COND_INITIALIZER;

static void* control_sys_thread(void* args){
    pthread_mutex_lock(&mutex_cond_cont_sys);

    struct timespec wake_time;

    motor_step_t motor_out;
    telescope_att_t cur_pos;

    while(1){

        pthread_cond_wait(&cond_cont_sys, &mutex_cond_cont_sys);

        clock_gettime(CLOCK_MONOTONIC, &wake_time);

        /* for loop for testing */
        //for(int ii=0; ii<10; ii++){
        while(get_mode() != RESET){

            kf_update(&cur_pos);

            pid_update(&cur_pos, &motor_out);

            step_az_alt(&motor_out);

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
