/* -----------------------------------------------------------------------------
 * Component Name: Gimbal
 * Parent Component: Control System
 * Author(s): 
 * Purpose: Provide an interface for the control of the gimbal motors.
 *
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
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

#include "gimbal.h"

static int fd_i2c;
int addr_az_alt = 0x70;
int addr_roll = 0x0F;

pthread_mutex_t mutex_i2c = PTHREAD_MUTEX_INITIALIZER;

int init_gimbal(void* args){

    fd_i2c = open("/dev/i2c-5", O_RDWR);
    if(fd_i2c == -1){
        logging(ERROR, "Gimbal", "Failed to open i2c-5 device: %m");
        return FAILURE;
    }

    return SUCCESS;
}

int step_az_alt(motor_step_t* steps){

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

    pthread_mutex_lock(&mutex_i2c);

    if(ioctl(fd_i2c, I2C_SLAVE, addr_az_alt) == -1){
        logging(ERROR, "Motor Cont", "Failed to set i2c slave address: %m");
        pthread_mutex_unlock(&mutex_i2c);
        return errno;
    }
    if(write(fd_i2c, &msg_az, 1) != 1){
        pthread_mutex_unlock(&mutex_i2c);
        return errno;
    }

    if(write(fd_i2c, &msg_alt, 1) != 1){
        pthread_mutex_unlock(&mutex_i2c);
        return errno;
    }

    pthread_mutex_unlock(&mutex_i2c);

    return SUCCESS;
}

int step_roll(motor_step_t* steps){

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

    pthread_mutex_lock(&mutex_i2c);

    if(ioctl(fd_i2c, I2C_SLAVE, addr_roll) == -1){
        logging(ERROR, "Motor Cont", "Failed to set i2c slave address: %m");
        pthread_mutex_unlock(&mutex_i2c);
        return errno;
    }
    if(write(fd_i2c, &msg, 1) != 1){
        return errno;
    }

    pthread_mutex_unlock(&mutex_i2c);

    return SUCCESS;
}
