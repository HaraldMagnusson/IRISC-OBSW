/* -----------------------------------------------------------------------------
 * Component Name: Global Utils
 * Author(s): 
 * Purpose: Define utilities and constants available to all components.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "global_utils.h"


int debug_mode = 1;


int init_global_utils( void ){
    return SUCCESS;
}

int init_submodules(const module_init_t *init_sequence, int module_count) {
    int ret;

    for(int i=0; i<module_count; ++i){
        ret = init_sequence[i].init();
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

    if (debug_mode == 0 && level == 0) return 0;

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
    vsprintf (buffer, format, args);
    // perror (buffer);
    va_end (args);

    fprintf(stderr, "%02d:%02d:%02d | %5.5s | %10.10s | %s\033[0m\n",
            hours, minutes, seconds,
            logging_levels[level], module_name, buffer);

    return SUCCESS;
}
