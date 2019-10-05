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
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <current_target/current_target.h>

#include "global_utils.h"
#include "stabilization.h"

void *stabilization_main_loop();
double saturate_output(double given_output);
int change_pid_values(int motor_id, double new_p, double new_i, double new_d);
int change_stabilization_mode(int on_off);
double motor_control_step(pid_values_t* current_pid_values,
                          pthread_mutex_t* pid_values_mutex,
                          control_variables_t* prev_vars,
                          control_variables_t* current_vars);

static pid_values_t current_az_pid_values;
static pid_values_t current_alt_pid_values;

static double motor_rate_threshold = 0.227;

/* Stabilization parameters */
static pid_values_t stab_az_pid_values = {
        .kp = 1
       ,.ki = 0
       ,.kd = 0
};
static pid_values_t stab_alt_pid_values = {
        .kp = 1
       ,.ki = 0.2
       ,.kd = 0
};

/* Tracking parameters */
static pid_values_t track_az_pid_values = {
        .kp = 1
       ,.ki = 0
       ,.kd = 0
};
static pid_values_t track_alt_pid_values = {
        .kp = 1
       ,.ki = 0.2
       ,.kd = 0
};

static pthread_mutex_t az_pid_values_mutex, alt_pid_values_mutex;

static control_variables_t az_prev_control_vars, az_current_control_vars,
                           alt_prev_control_vars, alt_current_control_vars;

static double az_expected_rate, alt_expected_rate;

static double stabilization_timestep = 0.001;
static double sim_time = 0;
//static int changer = 0;

// TODO: put it in the proper place
//static double rate = 0;

static telescope_att_t current_telescope_att;
static double az_motor_input, alt_motor_input;

//static double target_position = 0;
//static double current_position = 0;
//static double position_error = 0;

FILE *simdata;

int init_stabilization(void* args){
    simdata = fopen("/home/alarm/irisc-obsw/output/simdata.txt","w+");
    fprintf(simdata, "sim time,current pos,pos error,target pos,integral,derivative,proportional,pid output\n");

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

    change_stabilization_mode(0);
    pthread_t main_loop;
    pthread_create(&main_loop, NULL, stabilization_main_loop, NULL);

    return SUCCESS;
}

// TODO: Add watining for desired frequency
void *stabilization_main_loop() {
    usleep(10000);
    int i = 0;
    while(1) {
//        usleep(10);

        // Getting values from kalman filter and tracking subsystem
//        get_telescope_att(&current_telescope_att);

//        az_current_control_vars.current_position = az_prev_control_vars.current_position;
//        az_current_control_vars.current_position = current_telescope_att.az; // todo: uncomment this for final
//        alt_current_control_vars.current_position = current_telescope_att.alt;
//        get_tracking_angles(&az_current_control_vars.target_position, &alt_current_control_vars.target_position); // todo: uncomment this for final

        fprintf(stderr, "PID start: %.10f\n", az_prev_control_vars.pid_output);
        az_current_control_vars.current_position = az_prev_control_vars.pid_output;
        alt_current_control_vars.current_position = alt_prev_control_vars.pid_output;

        if(sim_time >= 1) az_current_control_vars.target_position = 20;
        else az_current_control_vars.target_position = 0;
//        az_current_control_vars.target_position = 20; // TODO: Delet this
        alt_current_control_vars.target_position = 20; // TODO: Delet this


        if(i == 0){
            az_prev_control_vars.position_error =  az_current_control_vars.target_position -  az_current_control_vars.current_position;
        }

        motor_control_step(&current_az_pid_values, &az_pid_values_mutex,
                           &az_prev_control_vars, &az_current_control_vars);
        motor_control_step(&current_alt_pid_values, &alt_pid_values_mutex,
                           &alt_prev_control_vars, &alt_current_control_vars);
        // todo: remove this for final

        // For simulation and testing
        logging(DEBUG, "Stabil", "--- az ---");
        logging(DEBUG, "Stabil", "Sim time\t %.10f", sim_time);
        logging(DEBUG, "Stabil", "Current poz:\t %.10f", az_current_control_vars.current_position);
        logging(DEBUG, "Stabil", "Target poz:\t %.10f", az_current_control_vars.target_position);
        logging(DEBUG, "Stabil", "Poz error:\t %.10f", az_current_control_vars.position_error);
        logging(DEBUG, "Stabil", "Integral:\t %.10f", az_current_control_vars.integral*current_az_pid_values.ki);
        logging(DEBUG, "Stabil", "Derivative:\t %.10f", az_current_control_vars.derivative*current_az_pid_values.kd);
        logging(DEBUG, "Stabil", "PID Output:\t %.10f", az_current_control_vars.pid_output);

        // For simulation and testing
//        az_current_control_vars.angular_rate += az_current_control_vars.pid_output * stabilization_timestep;
//        az_current_control_vars.current_position += az_current_control_vars.angular_rate * stabilization_timestep;
//        alt_current_control_vars.angular_rate += alt_current_control_vars.pid_output * stabilization_timestep;
//        alt_current_control_vars.current_position += alt_current_control_vars.angular_rate * stabilization_timestep;

        az_expected_rate = (az_current_control_vars.pid_output - az_current_control_vars.current_position);
        alt_expected_rate = (alt_current_control_vars.pid_output - alt_current_control_vars.current_position);

        // Output saturation
//        if(az_expected_rate > motor_rate_threshold*stabilization_timestep){
//            fprintf(stderr, "> ^ saturation upward\n");
//            az_current_control_vars.pid_output = az_current_control_vars.current_position + motor_rate_threshold*stabilization_timestep;
//        } else if (az_expected_rate < -motor_rate_threshold*stabilization_timestep) {
//            fprintf(stderr, "> v saturation downward\n");
//            az_current_control_vars.pid_output = az_current_control_vars.current_position - motor_rate_threshold*stabilization_timestep;
//        }
//
//        if(alt_expected_rate > motor_rate_threshold*stabilization_timestep){
//            alt_current_control_vars.pid_output = alt_current_control_vars.current_position + motor_rate_threshold*stabilization_timestep;
//        } else if (alt_expected_rate < -motor_rate_threshold*stabilization_timestep) {
//            alt_current_control_vars.pid_output = alt_current_control_vars.current_position - motor_rate_threshold*stabilization_timestep;
//        }

        fprintf(stderr, "er: %.10f\n", az_expected_rate);
        fprintf(stderr, "PID: %.10f\n", az_current_control_vars.pid_output);

//        alt_current_control_vars.angular_rate += alt_current_control_vars.pid_output * stabilization_timestep;

//        if(sim_time >= 10) change_pid_values(1, 0,0,0);

//        if(sim_time >= 43.82) exit(0); // TODO: Delete this after testing
        if(sim_time >= 100) exit(0); // TODO: Delete this after testing

//        fprintf(stderr, "\033[22D\033[8A");
        fprintf(simdata, "%.4f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                sim_time,
                az_current_control_vars.current_position,
                az_current_control_vars.position_error,
                az_current_control_vars.target_position,
                az_current_control_vars.integral*current_az_pid_values.ki,
                az_current_control_vars.derivative*current_az_pid_values.kd,
                az_current_control_vars.position_error*current_az_pid_values.kp,
                az_current_control_vars.pid_output);
        fflush(simdata);
        // TODO: here add some actual motor controlA


        az_prev_control_vars = az_current_control_vars;
        alt_prev_control_vars = alt_current_control_vars;
        fprintf(stderr, "PID end current: %.10f\n", az_current_control_vars.pid_output);
        fprintf(stderr, "PID end prev: %.10f\n", az_prev_control_vars.pid_output);
//        return NULL;
        sim_time = sim_time + stabilization_timestep;
        i++;
    }
}

double motor_control_step(pid_values_t* current_pid_values,
                        pthread_mutex_t* pid_values_mutex,
                        control_variables_t* prev_vars,
                        control_variables_t* current_vars) {

    // TODO: take into consideration that output and input are in different units
    //  that is, maybe torque vs angle
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

double saturate_output(double given_output){
    if (given_output >= motor_rate_threshold*stabilization_timestep) return motor_rate_threshold*stabilization_timestep;
    else if (given_output <= -motor_rate_threshold*stabilization_timestep) return -motor_rate_threshold*stabilization_timestep;
    else return given_output;
}

/* Changes pid parameters until next mode change
 *
 * First argument has to be motor id, that is:
 * 1 for azimuth control
 * 2 for altitude angle control.
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
        logging(ERROR, "Stabil", "change_pid_values: Wrong motor id.");
        return 1;
    }
}

/* Changes chosen mode pid parameters forever
 *
 * First argument has to be motor id, that is:
 * 1 for azimuth control
 * 2 for altitude angle control.
 *
 * Second argument has to be stabilization mode, that is:
 * 1 for target acquisition
 * 2 for stabilization.
 *
 * Second argument is mode - either
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
            logging(ERROR, "Stabil", "change_mode_pid_values: Wrong mode id.");
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
            logging(ERROR, "Stabil", "change_mode_pid_values: Wrong mode id.");
            pthread_mutex_unlock(&az_pid_values_mutex);
            return 1;
        }
        pthread_mutex_unlock(&alt_pid_values_mutex);
        return 0;
    } else {
        logging(ERROR, "Stabil", "change_mode_pid_values: Wrong motor id.");
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

        return 0;
    } else if (on_off == 0) {
        pthread_mutex_lock(&az_pid_values_mutex);
        current_az_pid_values = track_az_pid_values;
        pthread_mutex_unlock(&az_pid_values_mutex);

        pthread_mutex_lock(&alt_pid_values_mutex);
        current_alt_pid_values = track_alt_pid_values;
        pthread_mutex_unlock(&alt_pid_values_mutex);

        return 0;
    } else return 1;
}
