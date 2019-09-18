/* -----------------------------------------------------------------------------
 * Component Name: Data Queue
 * Parent Component: Img Processing
 * Author(s): Adam Smialek, William Eriksson
 * Purpose: Hold images until handler is ready.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "global_utils.h"

#include "data_queue.h"

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_queue_non_empty_cond = PTHREAD_COND_INITIALIZER;

static data_node *data_queue = NULL;

int init_data_queue(void) {
    return SUCCESS;
}

/**
 * Function to create a new node
 *
 * @param d     Filepath of data to be sent.
 * @param p     Priority of the data.
 */
data_node *new_node(char *f, int p, int type){
    data_node *temp = (data_node *) malloc(sizeof(data_node));
    temp->filepath = f;
    temp->priority = p;
    temp->type = type;
    temp->next = NULL;

    return temp;
}

/**
 * Function to check if the list is empty
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      1 if the list is empty, 0 if not.
 */
int is_empty(data_node **head) {
    return (*head) == NULL;
}

char *peek(data_node **head) {
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
struct node pop(data_node **head) {
    pthread_mutex_lock(&data_mutex);

    while (is_empty(head)) {
        pthread_cond_wait(&data_queue_non_empty_cond, &data_mutex);
    }

    data_node *temp = *head;
    (*head) = (*head)->next;
    struct node ret;
    ret.filepath = temp->filepath;
    ret.priority = temp->priority;
    ret.type = temp->type;

    free(temp);

    pthread_mutex_unlock(&data_mutex);
    return ret;
}

/**
 * Function to push node to the list according to its priority.
 *
 * @param head  Pointer to the first node of the linked list.
 * @param f     Filepath of data to be sent.
 * @param p     Priority of the data.
 */
void push(data_node **head, char *f, int p, int type) {
    pthread_mutex_lock(&data_mutex);
    data_node *start = (*head);

    if (!is_empty(head)) {
        // Create new node
        data_node *temp = new_node(f, p, type);
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
        *head = new_node(f, p, type);
    }

    pthread_mutex_unlock(&data_mutex);
    pthread_cond_signal(&data_queue_non_empty_cond);
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
int store_data_local(char *f, int p, int type) {
    push(&data_queue, f, p, type);
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
struct node read_data_queue() {
    struct node temp = pop(&data_queue);
    return temp;
}
