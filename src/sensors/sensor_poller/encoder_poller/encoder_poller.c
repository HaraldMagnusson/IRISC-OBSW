/* -----------------------------------------------------------------------------
 * Component Name: Encoder Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll the encoders for the current attitude relative to the gondola.
 * -----------------------------------------------------------------------------
 */

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <pthread.h>
#include <unistd.h>

#include "global_utils.h"
#include "encoder.h"

#define ENCODER_SAMPLE_TIME 10000000 /* unit: nanoseconds */

static int checksum_ctl(unsigned char str[2]);
static void proc(unsigned char str[2]);
static void* thread_func();

static struct timespec wake_time;
static pthread_t encoder_thread;

int init_encoder_poller( void ){

    int fd = wiringPiSPISetup(1, 1000000);
    int spi_mode = SPI_MODE_1;
    ioctl(fd, SPI_IOC_WR_MODE, &spi_mode);

    pthread_create(&encoder_thread, NULL, thread_func, NULL);

    return SUCCESS;
}

static void* thread_func(){

    unsigned char data[2];

    clock_gettime(CLOCK_MONOTONIC, &wake_time);
    wake_time.tv_sec += 2;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);

    while(1){
        wiringPiSPIDataRW(1, data, 2);

        if(checksum_ctl(data)){
            logging(WARN, "Encoder", "Incorrect checksum from encoder");
            encoder_out_of_date();
        }

        proc(data);

        wake_time.tv_nsec += ENCODER_SAMPLE_TIME;
        if(wake_time.tv_nsec >= 1000000000){
            wake_time.tv_sec++;
            wake_time.tv_nsec -= 1000000000;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
    }
}

void proc(unsigned char data[2]){

    unsigned short msg = (unsigned short)(data[0] & 0x3F) << 8 | (unsigned short)data[1];
    double ang = 360.0 * (double)msg / (double)0x4000;

    #ifdef ENCODER_DEBUG
        logging(DEBUG, "Encoder", "Encoder angle: %lf", ang);
    #endif

    encoder_t enc;
    enc.ra_motor = ang;

    set_encoder(&enc);
}

int checksum_ctl(unsigned char data[2]){

    unsigned char calc_odd, chk_odd, calc_even, chk_even;

    chk_odd = data[0] >> 7;
    chk_even = (data[0] >> 6) & 1;

    unsigned short msg = (unsigned short)(data[0] & 0x3F) << 8 | (unsigned short)data[1];

    calc_even = msg & 1;
    calc_odd = msg & 2;

    for(int ii=0; ii<6; ++ii){
        msg = msg >> 2;
        calc_odd ^= msg & 2;
        calc_even ^= msg & 1;
    }

    calc_odd = (calc_odd >> 1) ^ 1;
    calc_even ^= 1;

    if(calc_even == chk_even && calc_odd == chk_odd){
        return SUCCESS;
    }
    return FAILURE;
}

