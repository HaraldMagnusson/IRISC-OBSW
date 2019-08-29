/* -----------------------------------------------------------------------------
 * Component Name: Gyroscope Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll the gyroscopes for the current angular motion of the telescope.
 * -----------------------------------------------------------------------------
 */

#include <ftd2xx.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "global_utils.h"
#include "gyroscope.h"
#include "gpio.h"

#define SERIAL_NUM "FT2GZ6PG"
#define DATAGRAM_IDENTIFIER 0xAF
#define DATAGRAM_SIZE 65
#define GYRO_SAMPLE_TIME 5000000 /* unit: nanoseconds */

static void* thread_func(void* arg);

struct sensor_value
{
    signed int value:24;
} __attribute__ ((__packed__));

static pthread_t gyro_thread;
static FT_HANDLE fd;

int init_gyroscope_poller( void ){

    /* pin is left exported in case of crash */
    //gpio_unexport(PIN);

    int ret = gpio_export(GYRO_TRIG_PIN);
    if(ret != SUCCESS){
        return ret;
    }

    ret = gpio_direction(GYRO_TRIG_PIN, OUT);
    if(ret != SUCCESS){
        return ret;
    }

    gpio_write(GYRO_TRIG_PIN, HIGH);

    FT_STATUS stat;

    stat = FT_OpenEx(SERIAL_NUM, FT_OPEN_BY_SERIAL_NUMBER, &fd);
    if(stat != FT_OK){
        logging(ERROR, "GYRO", "Failed to initiate UART, error: %d",
                stat);
        return FAILURE;
    }

    stat = FT_SetBaudRate(fd, 921600);
    if(stat != FT_OK){
        logging(ERROR, "GYRO", "Failed to set baudrate for UART, error: %d",
                stat);
        return FAILURE;
    }

    stat = FT_SetTimeouts(fd, 4, 4);
    if(stat != FT_OK){
        logging(ERROR, "GYRO", "Failed to set timeout for UART, error: %d",
                stat);
        return FAILURE;
    }

    stat = FT_SetLatencyTimer(fd, 2);
    if(stat != FT_OK){
        logging(ERROR, "GYRO", "Failed to set latency timer for UART, "
                "error: %d", stat);
        return FAILURE;
    }

    ret = pthread_create(&gyro_thread, NULL, thread_func, (void*)&fd);
    if(ret != 0){
        logging(ERROR, "GYRO", "Failed to create gyro polling thread: "
                "%d, (%s)", ret, strerror(ret));
        return ret;
    }

    return SUCCESS;
}

static void* thread_func(void* arg){
    sleep(1);

    FT_HANDLE fd = *(FT_HANDLE*)arg;

    gyro_t gyro;
    struct timespec wake;
    struct sensor_value* sen;

    int ret;
    unsigned int bytes_read, bytes_available;
    unsigned char data[65], buffer[3];

    double rate[3];

    clock_gettime(CLOCK_MONOTONIC, &wake);
    wake.tv_sec += 1;

    ret = FT_Purge(fd, FT_PURGE_RX);
    if(ret != FT_OK){
        logging(WARN, "GYRO", "Failed to purge UART receive buffer: %d", ret);
    }

    while(1){

        /* do once loop to use break to jump to wait */
        do{
            /* create trigger pulse */
            gpio_write(GYRO_TRIG_PIN, LOW);
            usleep(1);
            gpio_write(GYRO_TRIG_PIN, HIGH);

            /* wait for UART conversion */
            usleep(2000);

            /* find start of datagram */
            do{
                FT_Read(fd, &data[0], 1, &bytes_read);
            } while(data[0] != DATAGRAM_IDENTIFIER);

            /* wait until full datagram is available */
            do{
                FT_GetQueueStatus(fd, &bytes_available);
                usleep(1);
            } while(bytes_available < DATAGRAM_SIZE-1);

            /* read full datagram */
            ret = FT_Read(fd, &data[1], 64, &bytes_read);
            if(ret != FT_OK){
                logging(WARN, "Gyro", "Reading datagram failed, "
                        "error: %d", ret);
                break;
            }

            /* check for datagram termination */
            if(     data[DATAGRAM_SIZE-2] != '\r' ||
                    data[DATAGRAM_SIZE-1] != '\n'){

                logging(WARN, "Gyro", "Incorrect datagram received");
                break;
            }

            /* check gyroscope data quality */
            if(data[10]){
                logging(WARN, "Gyro", "Bad gyroscope data quality: %2x", data[10]);
                gyro_out_of_date();
                break;
            }

            /* calculate gyroscope data */
            for(int ii=0; ii<3; ++ii){
                buffer[0] = data[3+3*ii];
                buffer[1] = data[2+3*ii];
                buffer[2] = data[1+3*ii];
                sen = (struct sensor_value*)(buffer);
                rate[ii] = (double)sen->value / (double)16384;
            }
            gyro.x = rate[0];
            gyro.y = rate[1];
            gyro.z = rate[2];

            set_gyro(&gyro);

            #if GYRO_DEBUG
                logging(DEBUG, "Gyro", "x: %+09.4lf\ty: %+09.4lf\tz: %+09.4lf",
                        gyro.x, gyro.y, gyro.z);
            #endif

        } while(0);

        wake.tv_nsec += GYRO_SAMPLE_TIME;
        if(wake.tv_nsec >= 1000000000){
            wake.tv_sec++;
            wake.tv_nsec -= 1000000000;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake, NULL);

    }
    return NULL;
}
