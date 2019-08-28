/* -----------------------------------------------------------------------------
 * Component Name: GPIO
 * Author(s): Harald Magusson
 * Purpose: Provide any initilisation and utilities needed for the GPIO.
 *          Pins use BCM numbering.
 * -----------------------------------------------------------------------------
 */

#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "global_utils.h"

#define PATH_MAX 40

static void close_unlock(int fd, pthread_mutex_t* mutex);

static pthread_mutex_t mutex_export;
static pthread_mutex_t mutex_unexport;
static pthread_mutex_t mutex_direction;
static pthread_mutex_t mutex_read;
static pthread_mutex_t mutex_write;

int init_gpio(void){

    int ret = pthread_mutex_init(&mutex_export, NULL);
    if( ret ){
        logging(ERROR, "GPIO",
                "The initialisation of the gpio export mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    ret = pthread_mutex_init(&mutex_unexport, NULL);
    if( ret ){
        logging(ERROR, "GPIO",
                "The initialisation of the gpio unexport mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    ret = pthread_mutex_init(&mutex_direction, NULL);
    if( ret ){
        logging(ERROR, "GPIO",
                "The initialisation of the gpio direction mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    ret = pthread_mutex_init(&mutex_read, NULL);
    if( ret ){
        logging(ERROR, "GPIO",
                "The initialisation of the gpio read mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    ret = pthread_mutex_init(&mutex_write, NULL);
    if( ret ){
        logging(ERROR, "GPIO",
                "The initialisation of the gpio write mutex failed: %d, (%s)",
                ret, strerror(ret));
        return ret;
    }

    wiringPiSetup();

    return SUCCESS;
}

int gpio_export(int pin){

    pthread_mutex_lock(&mutex_export);

    int fd = open("sys/class/gpio/export", O_WRONLY);
    if(fd == -1){
        logging(ERROR, "GPIO", "Failed to export pin: %d, (%s)",
                pin, strerror(errno));
        pthread_mutex_unlock(&mutex_export);
        return errno;
    }

    char buffer[3];
    size_t count = snprintf(buffer, 3, "%d", pin);
    int ret = write(fd, buffer, count);
    if(ret == -1){
        logging(ERROR, "GPIO", "Failed to export pin: %d, (%s)",
                pin, strerror(errno));
        close_unlock(fd, &mutex_export);
        return errno;
    }
    else if(ret != count){
        logging(ERROR, "GPIO", "Failed to export pin: %d", pin);
        close_unlock(fd, &mutex_export);
        return FAILURE;
    }

    close_unlock(fd, &mutex_export);
    return SUCCESS;
}

int gpio_unexport(int pin){

    pthread_mutex_lock(&mutex_unexport);

    int fd = open("sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1){
        logging(ERROR, "GPIO", "Failed to unexport pin: %d, (%s)",
                pin, strerror(errno));
        pthread_mutex_unlock(&mutex_unexport);
        return errno;
    }

    char buffer[3];
    size_t count = snprintf(buffer, 3, "%d", pin);
    int ret = write(fd, buffer, count);
    if(ret == -1){
        logging(ERROR, "GPIO", "Failed to unexport pin: %d, (%s)",
                pin, strerror(errno));
        close_unlock(fd, &mutex_unexport);
        return errno;
    }
    else if(ret != count){
        logging(ERROR, "GPIO", "Failed to unexport pin: %d", pin);
        close_unlock(fd, &mutex_unexport);
        return FAILURE;
    }

    close_unlock(fd, &mutex_unexport);
    return SUCCESS;
}

int gpio_direction(int pin, int dir){

    const char* directions = "in\0out";
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/sys/class/gpio/gpio%d/direction", pin);

    pthread_mutex_lock(&mutex_direction);

    int fd = open(path, O_WRONLY);
    if(fd == -1){
        logging(ERROR, "GPIO", "Failed to set direction of pin: %d, (%s)",
                pin, strerror(errno));
        pthread_mutex_unlock(&mutex_direction);
        return errno;
    }

    int count = IN == dir ? 2 : 3;
    int ret = write(fd, &directions[IN == dir ? 0 : 3], count);
    if(ret == -1){
        logging(ERROR, "GPIO", "Failed to set direction of pin: %d, (%s)",
                pin, strerror(errno));
        close_unlock(fd, &mutex_direction);
        return errno;
    }
    else if(ret != count){
        logging(ERROR, "GPIO", "Failed to set direction of pin: %d", pin);
        close_unlock(fd, &mutex_direction);
        return FAILURE;
    }

    close_unlock(fd, &mutex_direction);
    return SUCCESS;
}

int gpio_read(int pin, int* val){

    char path[PATH_MAX], value[2];
    snprintf(path, PATH_MAX, "/sys/class/gpio/gpio%d/value", pin);

    pthread_mutex_lock(&mutex_read);

    int fd = open(path, O_RDONLY);
    if(fd == -1){
        logging(ERROR, "GPIO", "Failed to read pin: %d, (%s)",
                pin, strerror(errno));
        pthread_mutex_unlock(&mutex_read);
        return errno;
    }

    int ret = read(fd, &value, 2);
    if(ret == -1){
        logging(ERROR, "GPIO", "Failed to read pin: %d, (%s)",
                pin, strerror(errno));
        close_unlock(fd, &mutex_read);
        return errno;
    }

    *val = atoi(value);
    close_unlock(fd, &mutex_read);
    return SUCCESS;
}

int gpio_write(int pin, int val){

    const char* values = "01";
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/sys/class/gpio/gpio%d/value", pin);

    pthread_mutex_lock(&mutex_write);

    int fd = open(path, O_WRONLY);
    if(fd == -1){
        logging(ERROR, "GPIO", "Failed to write to pin: %d, (%s)",
                pin, strerror(errno));
        pthread_mutex_unlock(&mutex_write);
        return errno;
    }

    int ret = write(fd, &values[LOW == val ? 0 : 1], 1);
    if(ret == 1){
        close_unlock(fd, &mutex_write);
        return SUCCESS;
    }
    else if(ret == -1){
        logging(ERROR, "GPIO", "Failed to write to pin: %d, (%s)",
                pin, strerror(errno));
        close_unlock(fd, &mutex_read);
        return errno;
    }

    close_unlock(fd, &mutex_read);
    return FAILURE;
}

static void close_unlock(int fd, pthread_mutex_t* mutex){
    close(fd);
    pthread_mutex_unlock(mutex);
}