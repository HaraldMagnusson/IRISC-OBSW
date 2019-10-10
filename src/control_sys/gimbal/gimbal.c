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

static int fd_i2c;
int addr_1 = 0x70;
int addr_2 = 0x0F;

int init_gimbal(void* args){

    fd_i2c = open("/dev/i2c-5", O_RDWR);
    if(fd_i2c == -1){
        printf("open: %m\n");
    }


    return SUCCESS;
}

int step_gimbal(int az_stepps, int el_stepps, int rt_stepps){

    char az_dir, az_buff, el_dir, el_buff, rt_dir, rt_buff;

    if(az_stepps>0){
        az_dir = 1;
        az_stepps = az_stepps*-1;
    } else {
        az_dir = 0;
    }
    if(el_stepps>0){
        el_dir = 1;
        el_stepps = el_stepps*-1;
    } else {
        el_dir = 0;
    }
    if(rt_stepps>0){
        rt_dir = 1;
        rt_stepps = rt_stepps*-1;
    } else {
        rt_dir = 0;
    }

    az_buff = (char)az_stepps;
    el_buff = (char)el_stepps;
    rt_buff = (char)rt_stepps;

    int ret = ioctl(fd_i2c, I2C_SLAVE, addr_1);
    if(ret == -1){
        printf("ioctl: %m\n");
    }

    write(fd_i2c, &az_buff, 1);
    write(fd_i2c, &el_buff, 1);

    ret = ioctl(fd_i2c, I2C_SLAVE, addr_2);
    if(ret == -1){
        printf("ioctl: %m\n");
    }

    write(fd_i2c, &rt_buff, 1);
    
    return SUCCESS;
}
