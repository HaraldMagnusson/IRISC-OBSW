/* -----------------------------------------------------------------------------
 * Component Name: Encoder
 * Parent Component: Sensors
 * Author(s): Harald Magnusson
 * Purpose: Keeps track of the telescope attitude relative to the gondola.
 *
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>

#include "global_utils.h"
#include "sensors.h"
#include "encoder.h"

static pthread_mutex_t mutex_encoder;
static encoder_t encoder_local;

int init_encoder(void* args){

    encoder_local.ra = 0;
    encoder_local.dec = 0;
    encoder_local.out_of_date = 1;

    int ret = pthread_mutex_init( &mutex_encoder, NULL );
    if( ret ){
        logging(MAIN_LOG, ERROR, "INIT",
                "The initialisation of the encoder mutex failed with code %d.\n",
                ret);
        return FAILURE;
    }

    return SUCCESS;
}

void get_encoder_local(encoder_t* encoder){

    pthread_mutex_lock(&mutex_encoder);

    encoder->ra = encoder_local.ra;
    encoder->dec = encoder_local.dec;
    encoder->out_of_date = encoder_local.out_of_date;

    pthread_mutex_unlock(&mutex_encoder);
}

void set_encoder(encoder_t* encoder){

    pthread_mutex_lock(&mutex_encoder);

    encoder_local.ra = encoder->ra;
    encoder_local.dec = encoder->dec;
    encoder_local.out_of_date = 0;

    pthread_mutex_unlock(&mutex_encoder);
}

void encoder_out_of_date(void){

    pthread_mutex_lock(&mutex_encoder);

    encoder_local.out_of_date = 1;

    pthread_mutex_unlock(&mutex_encoder);
}
