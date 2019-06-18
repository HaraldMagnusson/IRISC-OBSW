/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek
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
 * Node structure declaration.
 */
typedef struct node {
    int data;           // Data to be send as a telemetry.
    int priority;       // Lower values indicate higher priority
    struct node *next;  // Pointer to the node next on the list.

} downlink_node;

/**
 * Function to create a new node
 *
 * @param d     Data to be sent.
 * @param p     Priority of the data.
 */
downlink_node *new_node(int d, int p) {
    downlink_node *temp = (downlink_node *) malloc(sizeof(downlink_node));
    temp->data = d;
    temp->priority = p;
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

int peek(downlink_node **head) {
    if (!is_empty(head)) {
        return (*head)->data;
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
int pop(downlink_node **head) {
    pthread_mutex_lock(&downlink_mutex);

    while (is_empty(head)) {
        pthread_cond_wait(&queue_non_empty_cond, &downlink_mutex);
    }

    downlink_node *temp = *head;
    int data = (*head)->data;
    (*head) = (*head)->next;
    free(temp);

    pthread_mutex_unlock(&downlink_mutex);
    return data;
}

/**
 * Function to push node to the list according to its priority.
 *
 * @param head  Pointer to the first node of the linked list.
 * @param d     Data to be sent.
 * @param p     Priority of the data.
 */
void push(downlink_node **head, int d, int p) {
    pthread_mutex_lock(&downlink_mutex);
    downlink_node *start = (*head);

    if (!is_empty(head)) {
        // Create new node
        downlink_node *temp = new_node(d, p);
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
        *head = new_node(d, p);
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
int send_telemetry_local(int d, int p) {
    push(&downlink_queue, d, p);
    return SUCCESS;
}

/**
 * Return the data of the oldest message of the highest priority.
 * (This function exists solely for readability purposes, so some-
 * thing like `pop()` or `push()` won't appear somewhere without
 * context.)
 *
 * @return      Data from the first node of the linked list.
 */
int read_downlink_queue() {
    int data = pop(&downlink_queue);
    return data;
}
