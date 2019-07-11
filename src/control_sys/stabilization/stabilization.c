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
float saturate_output(float given_output);

float stabilization_timestep = 0.01;
float sim_time = 0;

float target_position = 0;
float current_position = 0;
float position_error = 0;
float last_position_error = 0;
float output = 0;

float integral = 0;
float derivative = 0;

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
    while(sim_time <= 20){
        sim_time = sim_time + stabilization_timestep;

        target_position = tracking_output_angle;
        current_position = filter_current_position;
        position_error = target_position - current_position;

        integral = integral + position_error*stabilization_timestep;
//        derivative = (position_error - last_position_error)/stabilization_timestep;
        derivative = (position_error - last_position_error);

        logging(DEBUG, "STABILIZATION", "current time: %f", sim_time);
        logging(DEBUG, "STABILIZATION", "current pos: %f", current_position);

        logging(DEBUG, "STABILIZATION", "target pos: %f", target_position);
        logging(DEBUG, "STABILIZATION", "integral: %f", integral);
        logging(DEBUG, "STABILIZATION", "derivative: %f", derivative);

        output = (Kp * position_error) + (Ki * integral) + (Kd * derivative);
        logging(DEBUG, "STABILIZATION", "before saturation: %f", output);
        output = saturate_output(output);
        logging(DEBUG, "STABILIZATION", "after saturation: %f", output);

        stabilization_output_angle = output;
        filter_current_position = current_position + output*stabilization_timestep;
//        fprintf(simdata, "%f,%f\n", sim_time, current_position);
        fprintf(simdata, "%f,%f,%f,%f,%f,%f,%f\n",
                sim_time, current_position, position_error, target_position, Ki*integral, Kd*derivative,Kp*position_error);
        fflush(simdata);
        fprintf(stderr, "\033[22D\033[7A");
//        usleep(stabilization_timestep*1000000);
        usleep(stabilization_timestep*10000);
    }
    return NULL;
}

float saturate_output(float given_output){
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
