/* -----------------------------------------------------------------------------
 * Component Name: GPS Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll the current gps position of the gondola.
 * -----------------------------------------------------------------------------
 */

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "global_utils.h"
#include "sensors.h"
#include "gps.h"

#define BUFFER_S 100

static int check_chksum(const unsigned char* str);
static unsigned char calc_chksum(const unsigned char* str, unsigned char* chksum_ascii);
static unsigned char nibble_to_ascii_hex(const unsigned char nibble);
static int divide_NMEA_str(const unsigned char* str, unsigned char NMEA_str_arr[30][20]);
static int process_gps(const unsigned char str[BUFFER_S]);
static void* gps_thread_func();
static float coord_conv(const unsigned char* str, int lon);

static unsigned char ch = 0xFF;
static unsigned char buffer[BUFFER_S];
static struct timespec wake_time;
static pthread_t gps_thread;
static int fd_spi12;


int init_gps_poller( void ){

    char* spi12 = "/dev/spidev1.2";

    fd_spi12 = open(spi12, O_RDONLY);

    if(fd_spi12 < 0){
        logging(ERROR, "GPS", "Failed to open spi device: %s", strerror(errno));
    }

    __u32 speed = 200000;
    ioctl(fd_spi12, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    buffer[0] = '$';

    pthread_create(&gps_thread, NULL, gps_thread_func, NULL);

    return SUCCESS;
}

static void* gps_thread_func(){
    int ii, ret = 0;

    clock_gettime(CLOCK_MONOTONIC, &wake_time);
    wake_time.tv_sec += 2;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);

    while(1){
        ii = 1;
        read(fd_spi12, &ch, 1);

        if(ch == '$'){

            do{
                buffer[ii] = 0xFF;
                read(fd_spi12, &buffer[ii], 1);
            } while(buffer[ii++] != '\r' && ii < BUFFER_S);

            buffer[ii-1] = '\0'; /* replace '\r' with '\0' */

            if(     buffer[3] == 'G' &&
                    buffer[4] == 'G' &&
                    buffer[5] == 'A'){

                #if GPS_DEBUG
                    logging(DEBUG, "GPS", "%s", buffer);
                #endif

                ret = process_gps(buffer);

                if(ret != SUCCESS){
                    gps_out_of_date();
                }

                wake_time.tv_sec += GPS_SAMPLE_TIME;
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_time, NULL);
            }
        }
    }
}

/* process_gps:
 * Accepts a GGA message, validate that the checksum is correct, and updates the
 * protected gps coordinates.
 *
 * input:
 *      str: GGA message following the NMEA standard
 *
 * return:
 *      SUCCESS: operation is successful
 *      FAILURE: checksum is incorrect or GPS receiver has bad connection
 *
 */
static int process_gps(const unsigned char str[BUFFER_S]){
    if( check_chksum(str) ){
        logging(WARN, "GPS", "Incorrect checksum in GPS data.");
        return FAILURE;
    }

    unsigned char NMEA_str_arr[30][20];
    divide_NMEA_str(str, NMEA_str_arr);

    /* Check data quality */
    if( NMEA_str_arr[6][0] == '0' ){
        logging(WARN, "GPS", "Bad GPS data quality.");
        return FAILURE;
    }

    gps_t gps;

    gps.lat = coord_conv(NMEA_str_arr[2], 0);
    gps.lon = coord_conv(NMEA_str_arr[4], 1);
    gps.alt = strtof((char*)NMEA_str_arr[9], NULL);

    set_gps(&gps);

    #if GPS_DEBUG
        logging(DEBUG, "GPS", "lat: %.3f, long: %.3f, alt: %.3f",
                gps.lat, gps.lon, gps.alt);
    #endif

    return SUCCESS;
}

/* coord_conv:
 * Converts a string containing latitude ("ddmm.mmmmm") or longitude ("dddmm.mmmmm")
 * to degrees as a float value.
 *
 * input:
 *      str: string of the form listed above
 *      lon: bool value, true if the input is longitude
 *
 * return:
 *      converted value in degrees
 */
static float coord_conv(const unsigned char* str, int lon){

    unsigned char deg_str[4] = "\0\0\0\0";
    int len;

    if(lon){
        len = 3;
    } else{
        len = 2;
    }

    for(int ii=0; ii<len; ++ii){
        deg_str[ii] = *str++;
    }

    float deg = strtof((char*)deg_str, NULL);
    float min = strtof((char*)str, NULL);

    deg += min/60;

    return deg;
}

/* check_chksum:
 * Checks if a string stored in str that follows the NMEA standard has a correct checksum.
 *
 * input:
 *      str: string following NMEA standard
 *
 * return:
 *      SUCCESS: checksum is correct
 *      FAILURE: checksum is incorrect
 */
static int check_chksum(const unsigned char* str){

    unsigned char chksum_ascii[3];
    calc_chksum(str, chksum_ascii);

    /* skip forward to character after '*' */
    while(*str++ != '*'){}

    if((str[0] == chksum_ascii[0]) && (str[1] == chksum_ascii[1])){
        return SUCCESS;
    }

    return FAILURE;
}

/* calc_chksum:
 * Calculates the checksum of a given string stored in str, between '$' and '*'
 * (excluding those characters).
 *
 * input:
 *      str: string beginning with '$' and containing a '*'
 *
 * output:
 *      chksum_ascii: the checksum in hex as a 3 byte string ending with NULL
 *
 * return:
 *      the byte value of the checksum
 */
static unsigned char calc_chksum(const unsigned char* str, unsigned char* chksum_ascii){
    unsigned char chksum = 0;

    /* skip '$' */
    ++str;

    while(*str != '*'){
        chksum ^= *str++;
    }

    /* extract most and least significant nibbles */
    unsigned char msn = chksum >> 4;
    unsigned char lsn = chksum & 0x0F;

    /* write into output */
    chksum_ascii[0] = nibble_to_ascii_hex(msn);
    chksum_ascii[1] = nibble_to_ascii_hex(lsn);
    chksum_ascii[2] = '\0';

    return chksum;
}

/* nibble_to_ascii_hex:
 * converts a nibble to the corresponding hex character.
 *
 * input:
 *      nibble: nibble to convert
 *
 * return:
 *      ascii value of hex character
 *      '\0' is returned for characters outside of the interval [0-15]
 */
static unsigned char nibble_to_ascii_hex(const unsigned char nibble){

    unsigned char ascii_hex;

    if((10 <= nibble) && (nibble <= 15)){
        ascii_hex = nibble - 10 + 'A';
    }
    else if((0 <= nibble) && (nibble <= 9)){
        ascii_hex = nibble + '0';
    }
    else{
        return '\0';
    }

    return ascii_hex;
}

/* divide_NMEA_str:
 * separates a string following the NMEA standard into each argument and
 * stores it in NMEA_str_arr.
 *
 * input:
 *      str: NMEA input string
 *
 * output:
 *      NMEA_str_arr: array to save arguments in
 *
 * return:
 *      SUCCESS: successful operation
 *
 */
static int divide_NMEA_str(const unsigned char* str, unsigned char NMEA_str_arr[30][20]){

    ++str; /* skip '$' */
    int k;

    for(int i=0; i<30; ++i){ /* loops for every argument */

        for(k=0; *str != '\0' && *str != ',' && *str != '*'; ++k){ /* writes each arguments */
            NMEA_str_arr[i][k] = *str++;
        }
        if(*str == ','){
            ++str;
        }
        else if(*str == '*'){

            NMEA_str_arr[i][k]= '\0';
            ++i;

            for(k=0; k<3; ++k){ /* write the checksum */
                NMEA_str_arr[i][k] = *str++;
            }
        }
        NMEA_str_arr[i][k]= '\0'; /* writes null character at end of string */
    }
    return SUCCESS;
}

