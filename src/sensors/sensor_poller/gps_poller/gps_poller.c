/* -----------------------------------------------------------------------------
 * Component Name: GPS Poller
 * Parent Component: Sensor Poller
 * Author(s): Harald Magnusson
 * Purpose: Poll the current gps position of the gondola.
 * -----------------------------------------------------------------------------
 */

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "global_utils.h"
#include "gps.h"

#define GPS_SAMPLE_TIME 4
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


int init_gps_poller( void ){
    wiringPiSetup();
    wiringPiSPISetup(0, 200000);

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
        ch = 0xFF;
        wiringPiSPIDataRW(0, &ch, 1);

        if(ch == '$'){

            do{
                buffer[ii] = 0xFF;
                wiringPiSPIDataRW(0, &buffer[ii], 1);
            } while(buffer[ii++] != '\r' && ii < BUFFER_S);

            buffer[ii-1] = '\0'; /* replace '\r' with '\0' */

            if(     buffer[3] == 'G' &&
                    buffer[4] == 'G' &&
                    buffer[5] == 'A'){

                for(int jj=0; jj<1; ++jj){
                    fprintf(stderr, "\033[A\033[2K");
                }

                logging(DEBUG, "GPS", "%s", buffer);
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

    set_gps(gps);

    return SUCCESS;
}

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


static int check_chksum(const unsigned char* str){
    /* check_chksum checks if a string stored in str that follows
     * the NMEA standard has a correct checksum.
     */

    unsigned char chksum_ascii[3];
    calc_chksum(str, chksum_ascii);

    /* skip forward to character after '*' */
    while(*str++ != '*'){}

    if((str[0] == chksum_ascii[0]) && (str[1] == chksum_ascii[1])){
        return SUCCESS;
    }

    return FAILURE;
}

static unsigned char calc_chksum(const unsigned char* str, unsigned char* chksum_ascii){
    /* calc_chksum calculates the checksum of a given
     * string stored in str, between '$' and '*'.
     */
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

static unsigned char nibble_to_ascii_hex(const unsigned char nibble){
    /* nibble_to_ascii_hex converts a nibble to the
     * corresponding hex character.
     */

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

static int divide_NMEA_str(const unsigned char* str, unsigned char NMEA_str_arr[30][20]){
    /* divide_NMEA_str separates a string following the NMEA standard into
     * each argument and stores it in NMEA_str_arr.
     */

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
