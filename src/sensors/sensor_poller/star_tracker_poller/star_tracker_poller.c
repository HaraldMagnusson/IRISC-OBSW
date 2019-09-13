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
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "global_utils.h"
#include "sensors.h"
#include "star_tracker.h"

#define TETRAPATH "Tetra/tetra.py"

static void irisc_tetra(float st_return[]);
static int call_tetra(float st_return[]);
static void* st_poller_thread(void* arg);
pid_t popen2(char* const * command, int *infp, int *outfp);

static pid_t py_pid;

static pthread_t st_poller_tid;

int init_star_tracker_poller(void* args){

    pthread_create(&st_poller_tid, NULL, st_poller_thread, NULL);

    return SUCCESS;
}

static void* st_poller_thread(void* arg){

    struct timespec wake;

    float st_return[4];

    clock_gettime(CLOCK_MONOTONIC, &wake);
    wake.tv_sec++;

    while(1){

        /* capture image */

        /* star tracker calculations */
        if(call_tetra(st_return)){
            st_out_of_date();
            continue;
        }

        /* move image to img queue dir */

        /* queue up image */




    }
    return NULL;
}


static int call_tetra(float st_return[]){

    star_tracker_t st;

    #ifdef ST_DEBUG
        struct timespec samp_0, samp, diff;
    #endif

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
        logging(WARN, "Star Tracker", "FoV = 0, lost in space failed");
        return FAILURE;
    }

    st.ra = st_return[0];
    st.dec = st_return[1];
    st.roll = st_return[2];

    set_star_tracker(&st);

    return SUCCESS;
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

    char* cmd[6] = {
        "chrt",
        "-f",
        "30",
        "python",
        TETRAPATH,
        NULL
    };

    int outfp = -1;
    py_pid = popen2(cmd, NULL, &outfp);
    waitpid(py_pid, NULL, 0);

    char buffer[100];
    int cnt = 0, ii;
    for(ii = 0; cnt < 4 && ii < 100; ++ii){
        read(outfp, &buffer[ii], 1);
        if(buffer[ii] == '\n'){
            cnt++;
        }
    }
    buffer[ii] = '\0';

    sscanf(buffer, "%*s %f\n" "%*s %f\n" "%*s %f\n" "%*s %f\n",
            &st_return[0], &st_return[1], &st_return[2], &st_return[3]);

    close(outfp);

/*
    FILE *fp = popen("sudo chrt -f 30 python "TETRAPATH, "r");

    for (int i = 0; i < 4; i++) {
        fscanf( fp, "%*s %f", &st_return[i]);
    }

    pclose(fp);
*/
    return;

}

pid_t get_star_tracker_pid(void){
    return py_pid;
}

#define READ 0
#define WRITE 1

pid_t popen2(char* const * command, int *infp, int *outfp){
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execvp(*command, command);
        perror("execvp");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];

    return pid;
}