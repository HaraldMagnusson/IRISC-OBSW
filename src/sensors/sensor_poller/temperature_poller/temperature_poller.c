/* -----------------------------------------------------------------------------
 * Component Name: Temperature Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll all thermometers for the current temperature status.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "global_utils.h"

static void* temp_poller_thread(void* args);
static void active_m(void);

static int fd_i2c;
static pthread_mutex_t mutex_i2c = PTHREAD_MUTEX_INITIALIZER;

int init_temperature_poller(void* args){

    fd_i2c = open("/dev/i2c-1", O_RDWR);
    if(fd_i2c == -1){
        logging(ERROR, "Temp poller", "Failed to open i2c-1 device: %m");
        return errno;
    }

    if(ioctl(fd_i2c, I2C_SLAVE, I2C_ADDR_HE) == -1){
        logging(ERROR, "Temp poller", "Failed to set i2c address: %m");
        return errno;
    }

    unsigned char setup_buf[3] = {0x02, 0x00, 0x58};
    if(write(fd_i2c, setup_buf, 3) != 3){
        logging(ERROR, "Temp poller", "Failed to setup config reg on ADC: %m");
        return errno;
    }

    return create_thread("temp_poller", temp_poller_thread, 20);
}

static void* temp_poller_thread(void* args){

    struct timespec wake;
    clock_gettime(CLOCK_MONOTONIC, &wake);

    while(1){
        active_m();

        wake.tv_sec += TEMP_SAMPLE_TIME;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);

    }

    return NULL;
}

static void active_m(void){

    unsigned char addrs[2] = {0x80, 0xA0};

    for(int ii=0; ii<2; ++ii){

        write(fd_i2c, &addrs[ii], 1);
        unsigned char buf[2];

        if(read(fd_i2c, buf, 2) != 2){
            printf("read: %m");
            continue;
        }
        unsigned short val = (0x0F & buf[0]) << 8 | buf[1];

        printf("V_in_%d:\t%02x%02x:\t%s\n",
                ii? 3:1, buf[0], buf[1], val > 0x0800 ? "high" : "low ");
    }
    printf("\033[2A");

}

/* check if the field rotator is on a given edge */
char fr_on_edge_ll(char edge){

    unsigned char addr_clkw = 0x80;
    unsigned char addr_ctclkw = 0xA0;
    unsigned char buf[2];

    pthread_mutex_lock(&mutex_i2c);

    if(edge){
        write(fd_i2c, &addr_clkw, 1);
    }
    else{
        write(fd_i2c, &addr_ctclkw, 1);
    }

    read(fd_i2c, buf, 2);

    pthread_mutex_unlock(&mutex_i2c);

    unsigned short val = (0x0F & buf[0]) << 8 | buf[1];
    return val > 0x0800;
}
