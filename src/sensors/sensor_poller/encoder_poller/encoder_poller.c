/* -----------------------------------------------------------------------------
 * Component Name: Encoder Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll the encoders for the current attitude relative to the gondola.
 * -----------------------------------------------------------------------------
 */

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "global_utils.h"
#include "sensors.h"
#include "encoder.h"
#include "encoder_poller.h"
#include "mode.h"

/* indicies for data arrays */
#define RA 0
#define DEC 1

static int checksum_ctl(unsigned char data[2][2]);
static int checksum_ctl_enc(unsigned char data[2]);
static void proc(unsigned char data[2][2], encoder_t* enc);
static void* thread_func(void* args);
static void active_m(void);

pthread_mutex_t mutex_cond_enc;
pthread_cond_t cond_enc;

static int fd_spi00, fd_spi01;
static FILE* encoder_log;

int init_encoder_poller(void* args){

    int ret;

    /* set up log file */
    char log_fn[100];

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/encoder.log");

    encoder_log = fopen(log_fn, "a");

    /* set up spi devices */
    char* spi00 = "/dev/spidev0.0";
    char* spi01 = "/dev/spidev0.1";
    __u32 spi_mode = SPI_MODE_1;
    __u32 speed = 1000000;

    fd_spi00 = open(spi00, O_RDONLY);
    if(fd_spi00 == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to open spidev0.0, %s",
                strerror(errno));
        return errno;
    }

    fd_spi01 = open(spi01, O_RDONLY);
    if(fd_spi00 == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to open spidev0.1, %s",
                strerror(errno));
        return errno;
    }

    ret = ioctl(fd_spi00, SPI_IOC_WR_MODE, &spi_mode);
    if(ret == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to set spi mode for spidev0.0, %s",
                strerror(errno));
        return errno;
    }

    ret = ioctl(fd_spi01, SPI_IOC_WR_MODE, &spi_mode);
    if(ret == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to set spi mode for spidev0.1, %s",
                strerror(errno));
        return errno;
    }

    ret = ioctl(fd_spi00, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(ret == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to set spi speed for spidev0.0, %s",
                strerror(errno));
        return errno;
    }

    ret = ioctl(fd_spi01, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(ret == -1){
        logging(MAIN_LOG, ERROR, "Encoder",
                "Failed to set spi speed for spidev0.1, %s",
                strerror(errno));
        return errno;
    }
    /* done setting up spi devices */

    return create_thread("encoder", thread_func, 25);
}

/* fetch a single sample from the encoder */
int enc_single_samp_ll(encoder_t* enc){

    unsigned char data[2][2];

    read(fd_spi00, data[RA], 2);
    read(fd_spi01, data[DEC], 2);

    if(checksum_ctl(data)){
        return FAILURE;
    }

    proc(data, enc);
    return SUCCESS;
}

static void* thread_func(void* args){

    struct timespec wake_time;

    pthread_mutex_lock(&mutex_cond_enc);

    while(1){

        pthread_cond_wait(&cond_enc, &mutex_cond_enc);

        clock_gettime(CLOCK_MONOTONIC, &wake_time);

        while(get_mode() != RESET){
            active_m();

            wake_time.tv_nsec += ENCODER_SAMPLE_TIME;
            if(wake_time.tv_nsec >= 1000000000){
                wake_time.tv_sec++;
                wake_time.tv_nsec -= 1000000000;
            }
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
        }
    }

    return NULL;
}

static encoder_t enc;
static void active_m(void){

    if(enc_single_samp(&enc)){
        encoder_out_of_date();
    }
    else{
        set_encoder(&enc);
    }
}

static int checksum_ctl(unsigned char data[2][2]){
    int ctl[2];

    for(int ii=0; ii<2; ++ii){
        ctl[ii] = checksum_ctl_enc(data[ii]);
    }

    if(ctl[RA]){
        if(ctl[DEC]){
            logging(MAIN_LOG, WARN, "Encoder",
                    "Incorrect checksum from encoders: ra & dec");
            return FAILURE;
        }
        logging(MAIN_LOG, WARN, "Encoder",
                "Incorrect checksum from encoder: ra");
        return FAILURE;
    }
    if(ctl[DEC]){
        logging(MAIN_LOG, WARN, "Encoder",
                "Incorrect checksum from encoder: dec");
        return FAILURE;
    }
    return SUCCESS;
}

static int checksum_ctl_enc(unsigned char data[2]){

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

static void proc(unsigned char data[2][2], encoder_t* enc){
    unsigned short data_s[2];
    double ang[2];

    for(int ii=0; ii<2; ++ii){
        data_s[ii] = (unsigned short)(data[ii][0] & 0x3F) << 8 | (unsigned short)data[ii][1];
        ang[ii] = 360.0 * (double)data_s[ii] / (double)0x4000;
    }

    #ifdef ENCODER_DEBUG
        logging(DEBUG, "Encoder", "ra: %lf \t dec: %lf", ang[RA], ang[DEC]);
    #endif

    enc->ra = ang[RA];
    enc->dec = ang[DEC];
}
