/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek, William Eriksson
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

/**
 * This module provides a priority queue for the telemetry module. It uses
 * linked list to solve the priority sorting, where the head of the list is
 * of a highest priority.
 *
 * @TODO Define MAX_LENGTH for the queue. If so, is it max size of the data
 *       in it, or max number of nodes?
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "global_utils.h"

#include "downlink_queue.h"

pthread_mutex_t downlink_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_non_empty_cond = PTHREAD_COND_INITIALIZER;

static downlink_node *downlink_queue = NULL;

int init_downlink_queue(void) {
    return SUCCESS;
}

/**
 * Function to create a new node
 *
 * @param d     Filepath of data to be sent.
 * @param p     Priority of the data.
 */
downlink_node *new_node(char *f, int p, int flag, unsigned short packets_sent){
    downlink_node *temp = (downlink_node *) malloc(sizeof(downlink_node));
    temp->filepath = f;
    temp->priority = p;
    temp->flag = flag;
    temp->packets_sent = packets_sent;
    temp->next = NULL;

    return temp;
}

/**
 * Function to check if the list is empty
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      1 if the list is empty, 0 if not.
 */
int is_empty(downlink_node **head) {
    return (*head) == NULL;
}

char *peek(downlink_node **head) {
    if (!is_empty(head)) {
        return (*head)->filepath;
    } else {
        return FAILURE;
    }
}

/**
 * Removes the element with the highest priority form the list and
 * return it's data value.
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      Data from the popped node.
 */
struct node pop(downlink_node **head) {
    pthread_mutex_lock(&downlink_mutex);

    while (is_empty(head)) {
        pthread_cond_wait(&queue_non_empty_cond, &downlink_mutex);
    }

    downlink_node *temp = *head;
    (*head) = (*head)->next;
    struct node ret;
    ret.filepath = temp->filepath;
    ret.flag = temp->flag;
    ret.packets_sent = temp->packets_sent;

    free(temp);

    pthread_mutex_unlock(&downlink_mutex);
    return ret;
}

/**
 * Function to push node to the list according to its priority.
 *
 * @param head  Pointer to the first node of the linked list.
 * @param f     Filepath of data to be sent.
 * @param p     Priority of the data.
 */
void push(downlink_node **head, char *f, int p, int flag, unsigned short packets_sent) {
    pthread_mutex_lock(&downlink_mutex);
    downlink_node *start = (*head);

    if (!is_empty(head)) {
        // Create new node
        downlink_node *temp = new_node(f, p, flag, packets_sent);
        // Special Case: The head of list has lesser
        // priority than new node. So insert new
        // node before head node and change head node.
        if ((*head)->priority > p) {
            // Insert New Node before head
            temp->next = *head;
            (*head) = temp;
        } else {
            // Traverse the list and find a
            // position to insert new node
            while (start->next != NULL &&
                   start->next->priority <= p) {
                start = start->next;
            }
            // Either at the ends of the list
            // or at required position
            temp->next = start->next;
            start->next = temp;
        }
    } else {
        *head = new_node(f, p, flag, packets_sent);
    }

    pthread_mutex_unlock(&downlink_mutex);
    pthread_cond_signal(&queue_non_empty_cond);
}

/**
 * Put data into the queue.
 * (This function exists solely for readability purposes, so some-
 * thing like `pop()` or `push()` won't appear somewhere without
 * context.)
 *
 * @param d     Data to be sent.
 * @param p     Priority of the data (lower `p` indicates higher
 *              priority).
 * @return      0
 */
int send_telemetry_local(char *f, int p, int flag, unsigned short packets_sent) {
    push(&downlink_queue, f, p, flag, packets_sent);
    return SUCCESS;
}

/**
 * Return the filepath of the oldest message of the highest priority.
 * (This function exists solely for readability purposes, so some-
 * thing like `pop()` or `push()` won't appear somewhere without
 * context.)
 *
 * @return      filepath from the first node of the linked list.
 */
struct node read_downlink_queue() {
    struct node temp = pop(&downlink_queue);
    return temp;
}
