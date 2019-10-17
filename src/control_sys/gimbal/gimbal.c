/* -----------------------------------------------------------------------------
 * Component Name: Gimbal
 * Parent Component: Control System
 * Author(s): Harald Magnusson
 * Purpose: Provide an interface for the control of the gimbal motors.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <pthread.h>
#include "control_sys.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

#include "gimbal.h"
#include "sensors.h"
#include "i2c.h"

unsigned char addr_az_alt = 8;
unsigned char addr_roll = 0x0F;

int init_gimbal(void* args){
    return SUCCESS;
}

int step_az_alt_local(motor_step_t* steps){

    int az = steps->az, alt = steps->alt;
    unsigned char az_dir, alt_dir;

    if(az < 0){
        az_dir = 0;
        az *= -1;
    }
    else{
        az_dir = 0x80;
    }

    if(alt < 0){
        alt_dir = 0;
        alt *= -1;
    }
    else{
        alt_dir = 0x80;
    }

    unsigned char msg_az  = az_dir  | (0x3F & az) | 0x40;
    unsigned char msg_alt = alt_dir | (0x3F & alt);

    unsigned buf[2] = {msg_az, msg_alt};

    if(write_i2c(5, addr_az_alt, buf, 2) != 2){
        return errno;
    }

    return SUCCESS;
}

int step_roll_local(motor_step_t* steps){

    int roll = steps->roll;
    unsigned char dir;

    if(roll < 0){
        dir = 0;
        roll *= -1;
    }
    else{
        dir = 0x80;
    }

    unsigned char msg = dir | (0x3F & roll);

    if(write_i2c(5, addr_roll, &msg, 1) != 1){
        return errno;
    }

    return SUCCESS;
}

/* Rotate the telescope to center of horizontal field of view, 45 deg up */
void center_telescope_l(void){

    move_alt_to(45);
    move_az_to(0);
}

/* Rotate the telescope to a target in az relative to gondola,
 * with an accuracy of 1 degree
 */
void move_az_to_l(double target){

    double err = 0;

    encoder_t enc;
    motor_step_t steps;

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    do{
        get_encoder(&enc);
        if(enc.out_of_date){
            continue;
        }

        err = target - enc.az;
        int sign  = err >= 0 ? 1 : -1;

        steps.az = 15 * sign;
        steps.alt = 0;
        step_az_alt(&steps);

        wake.tv_nsec += 10000000;
        if(wake.tv_nsec >= 1000000000){
            wake.tv_nsec -= 1000000000;
            wake.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);
    } while(fabs(err) < 1);

    steps.az = 0;
    step_az_alt(&steps);
}

/* Rotate the telescope to a target in alt relative to gondola,
 * with an accuracy of 1 degree
 */
void move_alt_to_l(double target){
    double err = 0;

    encoder_t enc;
    motor_step_t steps;

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    do{
        get_encoder(&enc);
        if(enc.out_of_date){
            continue;
        }

        err = target - enc.alt_ang;
        int sign  = err >= 0 ? 1 : -1;

        steps.alt = 15 * sign;
        steps.az = 0;
        step_az_alt(&steps);

        wake.tv_nsec += 10000000;
        if(wake.tv_nsec >= 1000000000){
            wake.tv_nsec -= 1000000000;
            wake.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);
    } while(fabs(err) < 1);

    steps.alt = 0;
    step_az_alt(&steps);
}
