/* -----------------------------------------------------------------------------
 * Component Name: Global Utils
 * Author(s): Adam Śmiałek, Harald Magnusson
 * Purpose: Define utilities and constants available to all components.
 *
 * -----------------------------------------------------------------------------
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>

#include "global_utils.h"

#define TOP_DIR_S 100

/* top_dir is the absolute path for the top directory
 * of the project evaluated at runtime.
 */
static char top_dir[TOP_DIR_S];
static const char* const top_dir_p = top_dir;

int init_global_utils(void* args){

    char* launch_arg = (char*) args;

    char* launch_dir = dirname(launch_arg);
    size_t dir_len = strlen(launch_dir);

    if(dir_len >= TOP_DIR_S){
        logging(ERROR, "INIT", "path to binary too long");
        return FAILURE;
    }

    /* absolute path */
    if(launch_arg[0] == '/'){
        strncpy(top_dir, launch_dir, TOP_DIR_S);
    }
    /* relative path */
    else if(launch_arg[0] == '.'){
        if(getcwd(top_dir, TOP_DIR_S) == NULL){
            logging(ERROR, "INIT", "Failed to fetch working directory, "
                    "%d (%s)", errno, strerror(errno));
            return FAILURE;
        }

        dir_len += strlen(top_dir);
        if(dir_len >= TOP_DIR_S){
            logging(ERROR, "INIT", "path to binary too long");
            return FAILURE;
        }
        strcat(top_dir, &launch_dir[1]);
    }

    /* remove bin at end of path
     * If run from the bin folder the result from the previous if statements
     * will end with "/bin/" otherwise it will end with "/bin".
     */
    dir_len = strlen(top_dir);
    if(top_dir[dir_len - 1] == '/'){
        top_dir[dir_len - 4] = '\0';
    }
    else{
        top_dir[dir_len - 3] = '\0';
    }

    return SUCCESS;
}

/* top_dir is the absolute path for the top directory
 * of the project evaluated at runtime.
 * get_top_dir returns a pointer to a string containing that directory.
 */
const char* const get_top_dir(void){
    return top_dir_p;
}

int init_submodules(const module_init_t *init_sequence, int module_count) {
    int ret;

    for(int i=0; i<module_count; ++i){
        ret = init_sequence[i].init(NULL);
        if( ret == SUCCESS ){
            logging(INFO, "INIT", " - Sub module \"%s\" initialised successfully.",
                    init_sequence[i].name);
        } else if( ret == FAILURE ){
            logging(ERROR, "INIT", " - Sub module \"%s\" FAILED TO INITIALISE, return value: %d",
                    init_sequence[i].name, ret);
            return ret;
        } else {
            logging(ERROR, "INIT", " - Sub module \"%s\" FAILED TO INITIALISE, return value: %d, %s",
                    init_sequence[i].name, ret, strerror(ret));
            return ret;
        }
    }

    return SUCCESS;
}

char logging_levels[5][7] =
        { "DEBUG",
          "INFO",
          "WARN",
          "ERROR",
          "CRIT"
        };

int logging(int level, char module_name[12],
            const char * format, ... ) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    int hours = local->tm_hour;
    int minutes = local->tm_min;
    int seconds = local->tm_sec;

    switch (level) {
        case 0 :
        case 1 :
            fprintf(stderr, "\033[0m");
            break;
        case 2 :
            fprintf(stderr, "\033[0;93m");
            break;
        case 3 :
            fprintf(stderr, "\033[0;91m");
            break;
        case 4 :
            fprintf(stderr, "\033[1;37;101m");
            break;
    }

    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 256, format, args);
    // perror (buffer);
    va_end (args);

    fprintf(stderr, "%02d:%02d:%02d | %5.5s | %10.10s | %s\033[0m\n",
            hours, minutes, seconds,
            logging_levels[level], module_name, buffer);

    return SUCCESS;
}

/* a call to pthread_create with additional thread attributes,
 * specifically priority
 */
int create_thread(char* comp_name, void* (*thread_func)(void*), int prio){

    pthread_attr_t attr;
    pthread_t tid;
    struct sched_param param;

    int ret = pthread_attr_init(&attr);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_attr_init for %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_attr_setstacksize of %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    param.sched_priority = prio;
    ret = pthread_attr_setschedparam(&attr, &param);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    ret = pthread_create(&tid, &attr, thread_func, NULL);
    if(ret != 0){
        fprintf(stderr,
            "Failed pthread_create of %s component. "
            "Return value: %d (%s)\n", comp_name, ret, strerror(ret));
        return ret;
    }

    pthread_setname_np(tid, comp_name);

    return SUCCESS;
}
