/* -----------------------------------------------------------------------------
 * Component Name: Controller
 * Parent Component: Tracking
 * Author(s): Adam Śmiałek
 * Purpose: Stabilise the telescope on the current target.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <pthread.h>
#include <control_sys.h>
#include <stdlib.h>

#include <unistd.h>

#include "global_utils.h"
#include "control_sys.h"
//#include "simple_sim.h"
//#include "rtwtypes.h"

//ExtU_simple_sim_T simple_sim_U;
//ExtY_simple_sim_T simple_sim_Y;

void *stabilization_main_loop();
double saturate_output(double given_output);

double stabilization_timestep = 0.01;
double sim_time = 0;

double target_position = 0;
double current_position = 0;
double position_error = 0;
double last_position_error = 0;
double output = 0;

double integral = 0;
double derivative = 0;

FILE *simdata;

int init_stabilization(void){
    stabilization_output_angle = 0;
    simdata = fopen("./src/control_sys/"
                    "stabilization/simdata.txt","w+");
    pthread_t main_loop;
    pthread_create(&main_loop, NULL,
                   stabilization_main_loop, NULL);
//    fclose(simdata);
    return SUCCESS;
}

void *stabilization_main_loop() {
    sleep(1);
    last_position_error = tracking_output_angle - filter_current_position;
    while(sim_time <= 20){
        sim_time = sim_time + stabilization_timestep;

//        target_position = tracking_output_angle;
        target_position = tracking_output_angle + sim_time/20;

        current_position = filter_current_position;
        position_error = target_position - current_position;

        integral = integral + position_error*stabilization_timestep;
        derivative = (position_error - last_position_error)/stabilization_timestep;
//        derivative = (position_error - last_position_error);

        last_position_error = position_error;

        logging(DEBUG, "STABILIZATION", "current time: %.10f", sim_time);
        logging(DEBUG, "STABILIZATION", "current pos: %.10f", current_position);

        logging(DEBUG, "STABILIZATION", "target pos: %.10f", target_position);
        logging(DEBUG, "STABILIZATION", "integral: %.10f", integral);
        logging(DEBUG, "STABILIZATION", "derivative: %.10f", derivative);

        output = (Kp * position_error) + (Ki * integral) + (Kd * derivative);
        logging(DEBUG, "STABILIZATION", "before saturation: %.10f", output);
//        output = saturate_output(output);
        logging(DEBUG, "STABILIZATION", "after saturation: %.10f", output);

        stabilization_output_angle = output;
        filter_current_position = current_position + output*stabilization_timestep;
//        fprintf(simdata, "%.10f,%.10f\n", sim_time, current_position);
        fprintf(simdata, "%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                sim_time, current_position, position_error, target_position, Ki*integral, Kd*derivative,Kp*position_error);
        fflush(simdata);
        fprintf(stderr, "\033[22D\033[7A");
//        usleep(stabilization_timestep*1000000);
        usleep(stabilization_timestep*10000);
    }
    return NULL;
}

double saturate_output(double given_output){
    if (given_output >= MOTOR_ANG_RATE_THRS) return MOTOR_ANG_RATE_THRS;
    else if (given_output <= -MOTOR_ANG_RATE_THRS) return -MOTOR_ANG_RATE_THRS;
    else return given_output;
}

//int stabilization_loop_step(){
//    printf("Hello world!\n");
//    int ans = 0;
//    simple_sim_initialize();
//
//    for (int i = 0; i < 10; i++)
//    {
//        simple_sim_U.In1 = i;
//        simple_sim_step();
//        int a = (int)simple_sim_U.In1;
//        int b = (int)simple_sim_Y.Out1;
//        printf("%d -> %d\n", a, b);
//        /* code */
//    }
//
//    simple_sim_terminate();
//    // ans = simple_sim_step();
//    printf("%x", ans);
//    return 0;
//}
