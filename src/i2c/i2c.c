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
