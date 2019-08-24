/* -----------------------------------------------------------------------------
 * Component Name: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground and
 *          send them when possible.
 *
 * -----------------------------------------------------------------------------
 */

/**
 * Module which has a responsibility of initialising downlink stream and
 * adding checksum to the sent info.
 *
 * @TODO Define the data types for the telemetry.
 * @TODO Define priority values for the possible telemetry types.
 */

#include <stdio.h>
#include <pthread.h>

#include "global_utils.h"

#include "downlink.h"
#include "downlink_queue.h"

#define MODULE_COUNT 2


/* This list controls the order of initialisation */
static const module_init_t init_sequence[MODULE_COUNT] = {
        {"downlink_queue",       &init_downlink_queue},
        {"downlink_", &init_downlink}
};

/*
// TODO: For test purposes only!
void *test_telemetry_generator() {
    for (int i = 0; i < 100; ++i) {
        send_telemetry_local(i, 0);
    }
    return NULL;
}

*/

/**
 * Infinite loop for sending telemetry via UDP.
 *
 * @return
 *
 * @TODO Move it to `init_telemetry`?
 * @TODO Redo it all!!!
 */
/*
void *telemetry_sender() {
    while (1) {
        char msg[64];
        int num = read_downlink_queue();
        sprintf(msg, "Message nr.%d", num);
        send_data_packet(msg);
    }
    return NULL;
}

*/

/**
 *
 * @return
 *
 * @TODO Add threads parameters and all that security stuff.
 */


int init_telemetry(void) {

    /* init whatever in this module */
    int ret = init_submodules(init_sequence, MODULE_COUNT);
    if (ret != SUCCESS) {
        return ret;
    }

  //  pthread_t generator;
  //  pthread_t sender;

  //  pthread_create(&sender, NULL, telemetry_sender, NULL);
  //  pthread_create(&generator, NULL, test_telemetry_generator, NULL);


/*  Test for queue

  send_telemetry("/tmp/test/ostkaka.csv", 5);
  send_telemetry("/home/rooo/bulle", 3);
  send_telemetry("/git/gud/404.cake", 7);

  char *temp;

  temp = read_downlink_queue();
  printf("First item: %s\n",temp);
  temp = read_downlink_queue();
  printf("Second item: %s\n",temp);
  temp = read_downlink_queue();
  printf("Third item: %s\n",temp);


  */

    return SUCCESS;
}



/**
 * Put data into the downlink queue.
 *
 * @param d     Data to be sent.
 * @param p     Priority of the data (lower `p` indicates higher
 *              priority).
 * @param flag  Indicate if data is a filepath(1) or string(0)
 *
 * @return      0
 */
int send_telemetry(char *filepath, int p, int flag, unsigned short packets_sent) {
    return send_telemetry_local(filepath, p, flag, packets_sent);
}
