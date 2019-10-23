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


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "global_utils.h"
#include "control_sys.h"
#include "gimbal.h"
#include "current_target.h"
#include "pid.h"
#include "sensors.h"
#include "telemetry.h"

double get_current_time();
double motor_control_step(pid_values_t* current_pid_values,
                          pthread_mutex_t* pid_values_mutex,
                          control_variables_t* prev_vars,
                          control_variables_t* current_vars);

static pid_values_t current_az_pid_values;
static pid_values_t current_alt_pid_values;

static double max_motor_ang_az = 0;
static double max_change_rate_az = 0;

static double max_motor_ang_alt = 0;
static double max_change_rate_alt = 0;

//static struct timespec wake_time;

/* Stabilization parameters */
static pid_values_t stab_az_pid_values = {
        .kp = 0.0673
       ,.ki = 0.05
       ,.kd = 0.0152
};
static pid_values_t stab_alt_pid_values = {
        .kp = 0.0673
       ,.ki = 0.05
       ,.kd = 0.0152
};

/* Tracking parameters */
static pid_values_t track_az_pid_values = {
        .kp = 0.0673
       ,.ki = 0.05
       ,.kd = 0.0152
};
static pid_values_t track_alt_pid_values = {
        .kp = 0.0673
       ,.ki = 0.05
       ,.kd = 0.0152
};

static pthread_mutex_t az_pid_values_mutex = PTHREAD_MUTEX_INITIALIZER,
        alt_pid_values_mutex = PTHREAD_MUTEX_INITIALIZER,
        az_control_vars_mutex = PTHREAD_MUTEX_INITIALIZER,
        alt_control_vars_mutex = PTHREAD_MUTEX_INITIALIZER;

static control_variables_t az_prev_control_vars, az_current_control_vars,
                           alt_prev_control_vars, alt_current_control_vars;

static double stabilization_timestep = (double)CONTROL_SYS_WAIT/1000000000;
static double sim_time = 0;
static double sim_start = 0;

//static int changer = 0; // Helper variable for simulation

/* factor for converting from angle to amount of steps */
static double step_per_deg = 0;
static encoder_t enc_hist[100];
static int stall_cntr_az = 100;
static int stall_cntr_alt = 100;

FILE *pid_az_log;
FILE *pid_alt_log;

int init_pid(void* args){
    char pid_log_fn[100];

    strcpy(pid_log_fn, get_top_dir());
    strcat(pid_log_fn, "output/logs/pid_az.log");

    pid_az_log = fopen(pid_log_fn, "a");
    if(pid_az_log == NULL){
        logging(ERROR, "PID", "Failed to open log file: %m");
    }

    strcpy(pid_log_fn, get_top_dir());
    strcat(pid_log_fn, "output/logs/pid_alt.log");

    pid_alt_log = fopen(pid_log_fn, "a");
    if(pid_alt_log == NULL){
        logging(ERROR, "PID", "Failed to open log file: %m");
    }

    //fprintf(pid_log, "log time,sim time,current pos,pos error,target pos,proportional,integral,derivative,pid output\n");

    az_prev_control_vars.current_position = 0;
    az_prev_control_vars.target_position = 0;
    az_prev_control_vars.position_error = 0;
    az_prev_control_vars.derivative = 0;
    az_prev_control_vars.integral = 0;
    az_prev_control_vars.pid_output = 0;

    alt_prev_control_vars.current_position = 0;
    alt_prev_control_vars.target_position = 0;
    alt_prev_control_vars.position_error = 0;
    alt_prev_control_vars.derivative = 0;
    alt_prev_control_vars.integral = 0;
    alt_prev_control_vars.pid_output = 0;

    /* factor for converting from angle to amount of steps */
    step_per_deg = (double)STEPS_PER_REVOLUTION * MICRO_STEP_FACTOR * GEARBOX_RATIO / 360.0;
    max_motor_ang_az = 25 / step_per_deg;
    max_change_rate_alt = 2 / step_per_deg;

    max_motor_ang_az = 15 / step_per_deg;
    max_change_rate_alt = 2 / step_per_deg;

    change_stabilization_mode(0);
//    pthread_t main_loop;
//    pthread_create(&main_loop, NULL, stabilization_main_loop, NULL);

    return SUCCESS;
}

char first_iteration = 1;
void pid_update(telescope_att_t* cur_att, motor_step_t* motor_out) {
//void *stabilization_main_loop() {
//    usleep(1000000); //1 sec
//    int i = 0;
//    while(1) {
//    usleep(10000); //0.01 sec TODO: Delete this, as in the end it's not a loop

    pthread_mutex_lock(&az_control_vars_mutex);
    pthread_mutex_lock(&alt_control_vars_mutex);

    // Getting values from Kalman filter and tracking subsystem
    az_current_control_vars.current_position = cur_att->az;
    alt_current_control_vars.current_position = cur_att->alt;
    get_tracking_angles(&az_current_control_vars.target_position, &alt_current_control_vars.target_position);
    encoder_t enc;
    get_encoder(&enc);
    az_current_control_vars.enc = enc.az;
    alt_current_control_vars.enc = enc.alt_ang;

    enc_hist[0] = enc;

    az_prev_control_vars.enc = enc_hist[4].az;
    alt_prev_control_vars.enc = enc_hist[4].alt_ang;

    // TODO: This is for simulation only
//    az_current_control_vars.current_position = az_prev_control_vars.pid_output;
//    alt_current_control_vars.current_position = alt_prev_control_vars.pid_output;
//    if(sim_time >= 1) az_current_control_vars.target_position = 20;
//    else az_current_control_vars.target_position = 0;
//    alt_current_control_vars.target_position = 20;
    // End of the sim block

    // Further initialization
    if(first_iteration){
        sim_start = get_current_time();
        az_prev_control_vars.time_in_seconds = sim_start;
        alt_prev_control_vars.time_in_seconds = sim_start;
        az_prev_control_vars.position_error =  0; //az_current_control_vars.target_position -  az_current_control_vars.current_position;
        alt_prev_control_vars.position_error =  0; //alt_current_control_vars.target_position -  alt_current_control_vars.current_position;

        first_iteration = 0;
    }

    sim_time = get_current_time();
    az_current_control_vars.time_in_seconds = sim_time - sim_start;
    alt_current_control_vars.time_in_seconds = sim_time - sim_start;

    // Main algorithm
    motor_control_step(&current_az_pid_values, &az_pid_values_mutex,
                       &az_prev_control_vars, &az_current_control_vars);
    motor_control_step(&current_alt_pid_values, &alt_pid_values_mutex,
                       &alt_prev_control_vars, &alt_current_control_vars);

    // azimuth anti windup
    double az_anti_windup = max_motor_ang_az / current_az_pid_values.ki;
    if(az_current_control_vars.integral > az_anti_windup){
        az_current_control_vars.integral = az_anti_windup;
    }
    else if(az_current_control_vars.integral < -az_anti_windup){
        az_current_control_vars.integral = -az_anti_windup;
    }

    // altitude anti windup
    double alt_anti_windup = max_motor_ang_alt / current_alt_pid_values.ki;
    if(alt_current_control_vars.integral > alt_anti_windup){
        alt_current_control_vars.integral = alt_anti_windup;
    }
    else if(alt_current_control_vars.integral < -alt_anti_windup){
        alt_current_control_vars.integral = -alt_anti_windup;
    }

    // azimuth pid output change rate limit
    if(fabs(az_current_control_vars.pid_output - az_prev_control_vars.pid_output) > max_change_rate_az){
        if( az_current_control_vars.pid_output > az_prev_control_vars.pid_output){
            az_current_control_vars.pid_output = az_prev_control_vars.pid_output + max_change_rate_az;
        }
        else{
            az_current_control_vars.pid_output = az_prev_control_vars.pid_output - max_change_rate_az;
        }
    }

    // altitude pid output change rate limit
    if(fabs(alt_current_control_vars.pid_output - alt_prev_control_vars.pid_output) > max_change_rate_alt){
        if( alt_current_control_vars.pid_output > alt_prev_control_vars.pid_output){
            alt_current_control_vars.pid_output = alt_prev_control_vars.pid_output + max_change_rate_alt;
        }
        else{
            alt_current_control_vars.pid_output = alt_prev_control_vars.pid_output - max_change_rate_alt;
        }
    }

    // Azimuth output saturation
    if(az_current_control_vars.pid_output > max_motor_ang_az){
        az_current_control_vars.pid_output = max_motor_ang_az;
    } else if (az_current_control_vars.pid_output < -max_motor_ang_az) {
        az_current_control_vars.pid_output = -max_motor_ang_az;
    }

    // Altitude output saturation
    //stabilization_timestep = alt_current_control_vars.time_in_seconds - alt_prev_control_vars.time_in_seconds;
    if(alt_current_control_vars.pid_output > max_motor_ang_alt){
        alt_current_control_vars.pid_output = max_motor_ang_alt;
    } else if (alt_current_control_vars.pid_output < -max_motor_ang_alt) {
        alt_current_control_vars.pid_output = -max_motor_ang_alt;
    }

    #ifdef PID_DEBUG
        logging(DEBUG, "PID", "Sim time\t %.10lf", az_current_control_vars.time_in_seconds);
    #endif

    //stall detection
    //az
    if((fabs(az_current_control_vars.integral) > 0.9 * az_anti_windup) &&
            (fabs(az_current_control_vars.enc - az_prev_control_vars.enc) < 0.05)){
        if(stall_cntr_az == 0){
            az_current_control_vars.pid_output = 0;
            stall_cntr_az = 100;
            send_telemetry("az motor has stalled", 1, 0, 0);
        }
        else{
            stall_cntr_az--;
        }
    }
    else{
        stall_cntr_az = 100;
    }

    //alt
    if((fabs(alt_current_control_vars.integral) > 0.9 * alt_anti_windup) &&
            (fabs(alt_current_control_vars.enc - alt_prev_control_vars.enc) < 0.05)){
        if(stall_cntr_alt == 0){
            alt_current_control_vars.pid_output = 0;
            stall_cntr_alt = 100;
            send_telemetry("alt motor has stalled", 1, 0, 0);
        }
        else{
            stall_cntr_alt--;
        }
    }
    else{
        stall_cntr_az = 100;
    }

    // Azimuth
    if(fabs(az_current_control_vars.position_error) < 0.02){
        az_current_control_vars.pid_output = 0;
    }

    // altitude
    if(fabs(alt_current_control_vars.position_error) < 0.02){
        alt_current_control_vars.pid_output = 0;
    }

    /* convert angle output to steps for output */
    motor_out->az = lround(step_per_deg * az_current_control_vars.pid_output);
    motor_out->alt = lround(step_per_deg * alt_current_control_vars.pid_output);

    logging_csv(pid_az_log, "%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%d,%d",
            az_current_control_vars.current_position,
            az_current_control_vars.position_error,
            az_current_control_vars.target_position,
            az_current_control_vars.position_error*current_az_pid_values.kp,
            az_current_control_vars.integral*current_az_pid_values.ki,
            az_current_control_vars.derivative*current_az_pid_values.kd,
            az_current_control_vars.pid_output,
            motor_out->az);

    logging_csv(pid_alt_log, "%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%d,%d",
            az_current_control_vars.current_position,
            az_current_control_vars.position_error,
            az_current_control_vars.target_position,
            az_current_control_vars.position_error*current_az_pid_values.kp,
            az_current_control_vars.integral*current_az_pid_values.ki,
            az_current_control_vars.derivative*current_az_pid_values.kd,
            az_current_control_vars.pid_output,
            motor_out->alt);

    az_prev_control_vars = az_current_control_vars;
    alt_prev_control_vars = alt_current_control_vars;

    for(int ii=99; ii>0; --ii){
        enc_hist[ii] = enc_hist[ii-1];
    }

    pthread_mutex_unlock(&az_control_vars_mutex);
    pthread_mutex_unlock(&alt_control_vars_mutex);

//        if(sim_time-sim_start >= 90.0 && changer == 0) {
//            pid_reset(); // TODO: Delete this after testing
//            change_stabilization_mode(1);
//            changer = 1;
//        }
//    sim_time = sim_time + stabilization_timestep;
//    }
}

/* Returns current time as a double, duh */
double get_current_time(){
    struct timeval tv;
    double time_in_micros = 0;
    double time_in_seconds = 0;

    gettimeofday(&tv,NULL);
    time_in_micros = 1000000 * (double)tv.tv_sec + (double)tv.tv_usec;
    time_in_seconds = (double)time_in_micros/(double)1000000;
    return time_in_seconds;
}

/* PID mathematical algorithm */
double motor_control_step(pid_values_t* current_pid_values,
                        pthread_mutex_t* pid_values_mutex,
                        control_variables_t* prev_vars,
                        control_variables_t* current_vars) {
    //stabilization_timestep = current_vars->time_in_seconds - prev_vars->time_in_seconds;

    //if (stabilization_timestep < 0.000001) stabilization_timestep = 0.000001;

    current_vars->position_error = current_vars->target_position - current_vars->current_position;
    current_vars->integral = prev_vars->integral + current_vars->position_error * stabilization_timestep;
    current_vars->derivative = (current_vars->position_error - prev_vars->position_error) / stabilization_timestep;

    pthread_mutex_lock(pid_values_mutex);
    current_vars->pid_output = (current_pid_values->kp * current_vars->position_error) +
                               (current_pid_values->ki * current_vars->integral) +
                               (current_pid_values->kd * current_vars->derivative);
    pthread_mutex_unlock(pid_values_mutex);
    return current_vars->pid_output;
}

/* Changes pid parameters until next mode change
 *
 * First argument has to be motor id, that is:
 *  1 for azimuth control
 *  2 for altitude angle control.
 */
int change_pid_values(int motor_id, double new_p, double new_i, double new_d){
    if (motor_id == 1) {
        pthread_mutex_lock(&az_pid_values_mutex);
        current_az_pid_values.kp = new_p;
        current_az_pid_values.ki = new_i;
        current_az_pid_values.kd = new_d;
        pthread_mutex_unlock(&az_pid_values_mutex);
        return 0;
    } else if (motor_id == 2) {
        pthread_mutex_lock(&alt_pid_values_mutex);
        current_alt_pid_values.kp = new_p;
        current_alt_pid_values.ki = new_i;
        current_alt_pid_values.kd = new_d;
        pthread_mutex_unlock(&alt_pid_values_mutex);
        return 0;
    } else {
        logging(ERROR, "PID", "change_pid_values: Wrong motor id.");
        return 1;
    }
}

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
int change_mode_pid_values(int motor_id, int mode_id, double new_p, double new_i, double new_d){
    if (motor_id == 1) {
        pthread_mutex_lock(&az_pid_values_mutex);
        if (mode_id == 1) {
            track_az_pid_values.kp = new_p;
            track_az_pid_values.ki = new_i;
            track_az_pid_values.kd = new_d;
        } else if (mode_id == 2) {
            stab_az_pid_values.kp = new_p;
            stab_az_pid_values.ki = new_i;
            stab_az_pid_values.kd = new_d;
        } else {
            logging(ERROR, "PID", "change_mode_pid_values: Wrong mode id.");
            pthread_mutex_unlock(&az_pid_values_mutex);
            return 1;
        }
        pthread_mutex_unlock(&az_pid_values_mutex);
        return 0;
    } else if (motor_id == 2) {
        pthread_mutex_lock(&alt_pid_values_mutex);
        if (mode_id == 1) {
            track_alt_pid_values.kp = new_p;
            track_alt_pid_values.ki = new_i;
            track_alt_pid_values.kd = new_d;
        } else if (mode_id == 2) {
            stab_alt_pid_values.kp = new_p;
            stab_alt_pid_values.ki = new_i;
            stab_alt_pid_values.kd = new_d;
        } else {
            logging(ERROR, "PID", "change_mode_pid_values: Wrong mode id.");
            pthread_mutex_unlock(&az_pid_values_mutex);
            return 1;
        }
        pthread_mutex_unlock(&alt_pid_values_mutex);
        return 0;
    } else {
        logging(ERROR, "PID", "change_mode_pid_values: Wrong motor id.");
        return 1;
    }
}

/* Sets pid parameters to and from stabilization mode
 *
 * 1 - on (ready for stabilization)
 * 0 - off (ready for tracking)
 */
int change_stabilization_mode(int on_off){
    if (on_off == 1) {
        pthread_mutex_lock(&az_pid_values_mutex);
        current_az_pid_values = stab_az_pid_values;
        pthread_mutex_unlock(&az_pid_values_mutex);

        pthread_mutex_lock(&alt_pid_values_mutex);
        current_alt_pid_values = stab_alt_pid_values;
        pthread_mutex_unlock(&alt_pid_values_mutex);

        logging(INFO, "PID", "Mode changed to stabilization.");
        return 0;
    } else if (on_off == 0) {
        pthread_mutex_lock(&az_pid_values_mutex);
        current_az_pid_values = track_az_pid_values;
        pthread_mutex_unlock(&az_pid_values_mutex);

        pthread_mutex_lock(&alt_pid_values_mutex);
        current_alt_pid_values = track_alt_pid_values;
        pthread_mutex_unlock(&alt_pid_values_mutex);

        logging(INFO, "PID", "Mode changed to tracking.");
        return 0;
    } else {
        logging(ERROR, "PID", "Wrong mode. Choose 1 for stabilization or 0 for tracking.");
        return 1;
    }
}

/* Resets the value for integral part and for the position error */
void pid_reset(){
    pthread_mutex_lock(&az_control_vars_mutex);
    az_prev_control_vars.position_error = 0;
    az_prev_control_vars.integral = 0;
    az_current_control_vars.position_error = 0;
    az_current_control_vars.integral = 0;
    pthread_mutex_unlock(&az_control_vars_mutex);

    pthread_mutex_lock(&alt_control_vars_mutex);
    alt_prev_control_vars.position_error = 0;
    alt_prev_control_vars.integral = 0;
    alt_current_control_vars.position_error = 0;
    alt_current_control_vars.integral = 0;
    pthread_mutex_unlock(&alt_control_vars_mutex);

    logging(INFO, "PID", "Resetting the integral part.");
}
