/* -----------------------------------------------------------------------------
 * Component Name: I2C
 * Author(s): Harald Magusson
 * Purpose: Provide initilisation and protection for the I2C buses.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>        // for mutexes
#include <errno.h>          // for errno
#include <sys/ioctl.h>      // for ioctl
#include <fcntl.h>          // for open
#include <unistd.h>         // for read/write
#include <linux/i2c-dev.h>  // for I2C_SLAVE

#include "global_utils.h"   // for logging, SUCCESS,  & FAILURE
#include "i2c.h"            // for write_i2c

static int fd_i2c_1 = -1;
static int fd_i2c_5 = -1;

static pthread_mutex_t mutex_i2c_1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_i2c_5 = PTHREAD_MUTEX_INITIALIZER;

int init_i2c(void* args){

    fd_i2c_1 = open("/dev/i2c-1", O_RDWR);
    if(fd_i2c_1 == -1){
        logging(ERROR, "I2C", "Failed to open i2c-1 device: %m");
        return FAILURE;
    }

    fd_i2c_5 = open("/dev/i2c-5", O_RDWR);
    if(fd_i2c_5 == -1){
        logging(ERROR, "I2C", "Failed to open i2c-5 device: %m");
        return FAILURE;
    }

    unsigned char setup_buf[3] = {0x02, 0x00, 0x58};
    if(write_i2c(1, I2C_ADDR_HE, setup_buf, 3) != 3){
        logging(ERROR, "I2C", "Failed to setup config reg on ADC: %m");
        return errno;
    }

    return SUCCESS;
}

ssize_t read_i2c(int dev_num, unsigned char addr, void* buf, size_t count){

    int fd;
    pthread_mutex_t mutex_i2c;

    switch(dev_num){
        case 1:
            fd = fd_i2c_1;
            mutex_i2c = mutex_i2c_1;
            break;
        case 5:
            fd = fd_i2c_5;
            mutex_i2c = mutex_i2c_5;
            break;
        default:
            errno = ENODEV;
            return FAILURE;
    }

    pthread_mutex_lock(&mutex_i2c);

    if(ioctl(fd, I2C_SLAVE, addr) == -1){
        pthread_mutex_unlock(&mutex_i2c);
        return FAILURE;
    }

    ssize_t ret = read(fd, buf, count);

    pthread_mutex_unlock(&mutex_i2c);

    return ret;
}

ssize_t write_i2c(int dev_num, unsigned char addr, const void* buf, size_t count){

    int fd;
    pthread_mutex_t mutex_i2c;

    switch(dev_num){
        case 1:
            fd = fd_i2c_1;
            mutex_i2c = mutex_i2c_1;
            break;
        case 5:
            fd = fd_i2c_5;
            mutex_i2c = mutex_i2c_5;
            break;
        default:
            errno = ENODEV;
            return FAILURE;
    }

    pthread_mutex_lock(&mutex_i2c);

    if(ioctl(fd, I2C_SLAVE, addr) == -1){
        pthread_mutex_unlock(&mutex_i2c);
        return FAILURE;
    }

    ssize_t ret = write(fd, buf, count);

    pthread_mutex_unlock(&mutex_i2c);

    return ret;
}

/* write_read_i2c first writes write_count bytes from write_buf to the address addr on the
 * i2c bus with the device number dev_num. Then read_count bytes are read into read_buf.
 *
 * The entire exchange is mutex protected with a mutex unique to the device number
 */
int write_read_i2c(int dev_num, unsigned char addr, const void* write_buf,
        size_t write_count, void* read_buf, size_t read_count,
        ssize_t* write_ret, ssize_t* read_ret){

    *write_ret = -2;
    *read_ret = -2;

    int fd;
    pthread_mutex_t mutex_i2c;

    switch(dev_num){
        case 1:
            fd = fd_i2c_1;
            mutex_i2c = mutex_i2c_1;
            break;
        case 5:
            fd = fd_i2c_5;
            mutex_i2c = mutex_i2c_5;
            break;
        default:
            errno = ENODEV;
            return FAILURE;
    }

    pthread_mutex_lock(&mutex_i2c);

    if(ioctl(fd, I2C_SLAVE, addr) == -1){
        pthread_mutex_unlock(&mutex_i2c);
        return FAILURE;
    }

    *write_ret = write(fd, write_buf, write_count);
    if(*write_ret != write_count){
        pthread_mutex_unlock(&mutex_i2c);
        return FAILURE;
    }

    *read_ret = read(fd, read_buf, read_count);
    if(*read_ret != read_count){
        pthread_mutex_unlock(&mutex_i2c);
        return FAILURE;
    }

    pthread_mutex_unlock(&mutex_i2c);

    return SUCCESS;
}

/* check if the field rotator is on a given edge */
char fr_on_edge(char edge){

    unsigned char addr_clkw = 0x80;
    unsigned char addr_ctclkw = 0xA0;
    unsigned char buf[2];

    if(edge){
        write_i2c(1, I2C_ADDR_HE, &addr_clkw, 1);
    }
    else{
        write_i2c(1, I2C_ADDR_HE, &addr_ctclkw, 1);
    }

    read_i2c(1, I2C_ADDR_HE, buf, 2);

    unsigned short val = (0x0F & buf[0]) << 8 | buf[1];
    return val < 0x0800;
}
