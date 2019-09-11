/* -----------------------------------------------------------------------------
 * Component Name: Star Tracker Poller
 * Parent Component: Sensor Poller
 * Author(s): Niklas Ulfvarson, Harald Magnusson
 * Purpose: Capture images with the guiding camera and perform necessary
 *          calculations to acquire the absolute attitude of the telescope.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "global_utils.h"

#define TETRAPATH "Tetra/tetra.py"

static void irisc_tetra(float st_return[]);
static void* st_poller_thread(void* arg);

static pthread_t st_poller_id;

int init_star_tracker_poller( void ){

    pthread_create(&st_poller_id, NULL, st_poller_thread, NULL);

    return SUCCESS;
}

static void* st_poller_thread(void* arg){

    struct timespec wake;

    float st_return[4];

    #ifdef ST_DEBUG
        struct timespec samp_0, samp, diff;
    #endif

    clock_gettime(CLOCK_MONOTONIC, &wake);
    wake.tv_sec++;

    while(1){
        #ifdef ST_DEBUG
            clock_gettime(CLOCK_MONOTONIC, &samp_0);
            irisc_tetra(st_return);
            clock_gettime(CLOCK_MONOTONIC, &samp);

            diff.tv_sec = samp.tv_sec - samp_0.tv_sec;
            diff.tv_nsec = samp.tv_nsec - samp_0.tv_nsec;
            if(diff.tv_nsec < 0){
                diff.tv_sec--;
                diff.tv_nsec += 1000000000;
            }

            logging(DEBUG, "Star Tracker", "Star tracker sample time: %ld.%09ld s",
                    diff.tv_sec, diff.tv_nsec);
            logging(DEBUG, "Star Tracker", "Output: %f, %f, %f, %f",
                    st_return[0], st_return[1], st_return[2], st_return[3]);
        #else
            irisc_tetra(st_return);
        #endif

        if(st_return[3] == 0){
            logging(WARN, "Star Tracker", "FoV = 0");
            continue;
        }
    }
    return NULL;
}



/*
 * Purpose: This is the interface to run the Tetra python program from the
 *          main OBSW.
 * Usage: A float array of length 4 i passed, and values from the star tracker
 *          is returned in this array in order:
 *          0: RA
 *          1: Dec
 *          2: Roll
 *          3: FoV
 *
 *          The name of the parameter is printed by tetra, but then ignored.
 *          If no attitude could be calculated all of these will be 0. This
 *          can obviously not happen if an attitude is calculated, as FoV will
 *          always have a positive non-zero value.
 */
static void irisc_tetra(float st_return[]) {


    FILE *fp = popen("sudo chrt -f 30 python "TETRAPATH, "r");

    for (int i = 0; i < 4; i++) {
        fscanf( fp, "%*s %f", &st_return[i]);
    }

    pclose(fp);
    return;

}
