/* -----------------------------------------------------------------------------
 * Component Name: Mode
 * Author(s): Harald Magnusson
 * Purpose: Store the current state of the software.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "mode.h"

static pthread_mutex_t mutex_mode;
static char mode;

static const char* modes[] = {
    "NORMAL",
    "SLEEP",
    "RESET",
    "WAKE"
};

int init_mode(void* args){

    mode = SLEEP;

    int res = pthread_mutex_init(&mutex_mode, NULL);
    if( res ){
        logging(ERROR, "MODE",
            "The initialisation of the mode mutex failed with code %d.",
            res);
        return FAILURE;
    }

    return SUCCESS;
}

void set_mode(char ch){

    pthread_mutex_lock( &mutex_mode );
    mode = ch;
    logging(INFO, "MODE", "Entering %s mode", modes[(size_t)ch]);
    pthread_mutex_unlock( &mutex_mode );

}

char get_mode(void){

    char ch;

    pthread_mutex_lock( &mutex_mode );
    ch = mode;
    pthread_mutex_unlock( &mutex_mode );

    return ch;
}
