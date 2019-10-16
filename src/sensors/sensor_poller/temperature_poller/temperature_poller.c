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

#if 0
    write(fd_i2c, "\x80", 1);

    unsigned char buf[2];
    read(fd_i2c, buf, 2);

    unsigned short val = (0x0F & buf[0]) << 8 | buf[1];

    printf("V_in_1:\t%02x%02x:\t%s\n", buf[0], buf[1], val > 0x0800 ? "high" : "low ");

    write(fd_i2c, "\xA0", 1);
    read(fd_i2c, buf, 2);

    val = (0x0F & buf[0]) << 8 | buf[1];

    printf("V_in_3:\t%02x%02x:\t%s\n\033[2A", buf[0], buf[1], val > 0x0800 ? "high" : "low ");
#elif 0
    write(fd_i2c, "\x70", 1);

    unsigned char buf[4];
    read(fd_i2c, buf, 4);
    printf("%02x,%02x,%02x,%02x\n", buf[0], buf[1], buf[2], buf[3]);

    unsigned short vals[2];
    for(int ii=0; ii<2; ++ii){
        vals[ii] = (0x0F & buf[2*ii]) << 8 | buf[2*ii+1];

        printf("V_in_%d:\t%02x%02x:\t%s\n",
                ii? 3:1, buf[2*ii], buf[2*ii+1], vals[ii] > 0x0800 ? "high" : "low ");
    }
    //printf("\033[2A");
#else

    unsigned char addrs[2] = {0x80, 0xA0};

    unsigned char buf[2];
    unsigned short val;
    for(int ii=0; ii<2; ++ii){
        write(fd_i2c, &addrs[ii], 1);
        if(read(fd_i2c, buf, 2) != 2){
            printf("read: %m");
            continue;
        }
        val = (0x0F & buf[0]) << 8 | buf[1];

        printf("V_in_%d:\t%02x%02x:\t%s\n",
                ii? 3:1, buf[0], buf[1], val > 0x0800 ? "high" : "low ");
    }
    printf("\033[2A");

#endif
}
