/* -----------------------------------------------------------------------------
 * Component Name: Global Utils
 * Author(s): 
 * Purpose: Define utilities and constants available to all components.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "global_utils.h"

int init_global_utils( void ){
    return SUCCESS;
}

int init_submodules(const module_init_t *init_sequence, int module_count) {
    int ret;

    for(int i=0; i<module_count; ++i){
        ret = init_sequence[i].init();
        if( ret == SUCCESS ){
            fprintf(stderr, "\tSub module \"%s\" initialised successfully.\n",
                    init_sequence[i].name);
        } else if( ret == FAILURE ){
            fprintf(stderr, "\tSub module \"%s\" FAILED TO INITIALISE, return value: %d\n",
                    init_sequence[i].name, ret);
            return ret;
        } else {
            fprintf(stderr, "\tSub module \"%s\" FAILED TO INITIALISE, return value: %d, %s\n",
                    init_sequence[i].name, ret, strerror(ret));
            return ret;
        }
    }

    return SUCCESS;
}
