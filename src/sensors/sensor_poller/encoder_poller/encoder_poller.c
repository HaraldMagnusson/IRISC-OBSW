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
#include "telemetry.h"

/* indicies for data arrays */
#define AZ 0
#define ALT_ANG 1

static int checksum_ctl(unsigned char data[2][2]);
static int checksum_ctl_enc(unsigned char data[2]);
static void proc(unsigned char data[2][2], encoder_t* enc);
static void* thread_func(void* args);
static void active_m(void);
static int read_offsets(void);

pthread_mutex_t mutex_cond_enc;
pthread_cond_t cond_enc;

static int fd_spi00, fd_spi01;

static double az_offset = 0, alt_offset = 0;

static FILE* encoder_log;

int init_encoder_poller(void* args){

    int ret;

    /* set up log file */
    char log_fn[100];

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/encoder.log");

    encoder_log = fopen(log_fn, "a");
    if(encoder_log == NULL){
        logging(ERROR, "Encoder",
                "Failed to open encoder log file: %m");
        return errno;
    }

    if(read_offsets()){
        return errno;
    }

    /* set up spi devices */
    char* spi00 = "/dev/spidev0.0";
    char* spi01 = "/dev/spidev0.1";
    __u32 spi_mode = SPI_MODE_1;
    __u32 speed = 50000;

    fd_spi00 = open(spi00, O_RDONLY);
    if(fd_spi00 == -1){
        logging(ERROR, "Encoder",
                "Failed to open spidev0.0: %m");
        return errno;
    }

    fd_spi01 = open(spi01, O_RDONLY);
    if(fd_spi00 == -1){
        logging(ERROR, "Encoder",
                "Failed to open spidev0.1: %m");
        return errno;
    }

    ret = ioctl(fd_spi00, SPI_IOC_WR_MODE, &spi_mode);
    if(ret == -1){
        logging(ERROR, "Encoder",
                "Failed to set spi mode for spidev0.0: %m");
        return errno;
    }

    ret = ioctl(fd_spi01, SPI_IOC_WR_MODE, &spi_mode);
    if(ret == -1){
        logging(ERROR, "Encoder",
                "Failed to set spi mode for spidev0.1: %m");
        return errno;
    }

    ret = ioctl(fd_spi00, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(ret == -1){
        logging(ERROR, "Encoder",
                "Failed to set spi speed for spidev0.0: %m");
        return errno;
    }

    ret = ioctl(fd_spi01, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(ret == -1){
        logging(ERROR, "Encoder",
                "Failed to set spi speed for spidev0.1: %m");
        return errno;
    }
    /* done setting up spi devices */

    return create_thread("encoder", thread_func, 25);
}

static int read_offsets(void){

    char fn[100];

    /* az offset */
    FILE* encoder_offset_az = NULL;
    strcpy(fn, get_top_dir());
    strcat(fn, "output/enc_az_offset.log");

    encoder_offset_az = fopen(fn, "r");
    if(encoder_offset_az == NULL){
        logging(ERROR, "Encoder",
                "Failed to open encoder az offset file: %m");
        return errno;
    }

    fread((void*)&az_offset, sizeof(double), 1, encoder_offset_az);
    fclose(encoder_offset_az);

    /* alt offset */
    FILE* encoder_offset_alt = NULL;
    strcpy(fn, get_top_dir());
    strcat(fn, "output/enc_alt_offset.log");

    encoder_offset_alt = fopen(fn, "r");
    if(encoder_offset_alt == NULL){
        logging(ERROR, "Encoder",
                "Failed to open encoder alt offset file: %m");
        return errno;
    }

    fread((void*)&alt_offset, sizeof(double), 1, encoder_offset_alt);
    fclose(encoder_offset_alt);

    return SUCCESS;
}

/* fetch a single sample from the encoder */
int enc_single_samp_ll(encoder_t* enc){

    unsigned char data[2][2];

    read(fd_spi00, data[AZ], 2);
    read(fd_spi01, data[ALT_ANG], 2);

    if(checksum_ctl(data)){
        errno = EIO;
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

    if(ctl[AZ]){
        if(ctl[ALT_ANG]){
            logging(WARN, "Encoder", "Incorrect checksum from encoders: az & alt_ang");
            return FAILURE;
        }
        logging(WARN, "Encoder", "Incorrect checksum from encoder: az");
        return FAILURE;
    }
    if(ctl[ALT_ANG]){
        logging(WARN, "Encoder", "Incorrect checksum from encoder: alt_ang");
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

/* set offsets for the azimuth and altitude angle encoders */
int set_offsets(void){

    encoder_t enc;
    if(enc_single_samp_ll(&enc)){
        return errno;
    }

    /* add the previous offsets to get the original encoder output */
    enc.az += az_offset;
    enc.alt_ang += alt_offset;

    /* store new offsets */
    az_offset = enc.az;
    alt_offset = enc.alt_ang;

    char fn[100];

    /* storing az in file */
    FILE* encoder_offset_az = NULL;
    strcpy(fn, get_top_dir());
    strcat(fn, "output/enc_az_offset.log");

    encoder_offset_az = fopen(fn, "w");
    if(encoder_offset_az == NULL){
        logging(ERROR, "Encoder",
                "Failed to open encoder az offset file: %m");
        return errno;
    }

    fwrite((void*)&az_offset, sizeof(double), 1, encoder_offset_az);
    fclose(encoder_offset_az);

    /* storing alt in file */
    FILE* encoder_offset_alt = NULL;
    strcpy(fn, get_top_dir());
    strcat(fn, "output/enc_alt_offset.log");

    encoder_offset_alt = fopen(fn, "w");
    if(encoder_offset_alt == NULL){
        logging(ERROR, "Encoder",
                "Failed to open encoder alt offset file: %m");
        return errno;
    }

    fwrite((void*)&alt_offset, sizeof(double), 1, encoder_offset_alt);
    fclose(encoder_offset_alt);

    char buffer[100];
    snprintf(buffer, 100, "Encoder offsets set to %lg az, %lg alt",
            az_offset, alt_offset);
    send_telemetry(buffer, 1, 0, 0);

    return SUCCESS;
}

static void proc(unsigned char data[2][2], encoder_t* enc){
    unsigned short data_s[2];
    double ang[2];

    for(int ii=0; ii<2; ++ii){
        data_s[ii] = (unsigned short)(data[ii][0] & 0x3F) << 8 | (unsigned short)data[ii][1];
        ang[ii] = 360.0 * (double)data_s[ii] / (double)0x4000;
    }

    logging_csv(encoder_log, "%010.6lf,%010.6lf", ang[AZ], ang[ALT_ANG]);

    #ifdef ENCODER_DEBUG
        logging(DEBUG, "Encoder", "ra: %lf \t dec: %lf", ang[AZ], ang[ALT_ANG]);
    #endif

    enc->az = ang[AZ] - az_offset;
    enc->alt_ang = 360 - ang[ALT_ANG] - alt_offset;
}
