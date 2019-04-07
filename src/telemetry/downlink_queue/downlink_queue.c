/* -----------------------------------------------------------------------------
 * Component Name: Downlink Queue
 * Parent Component: Telemetry
 * Author(s): Adam Smialek
 * Purpose: Provide a queue for telemetry messages to be sent to ground.
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>

#include "global_utils.h"

/*!
 * This module provides a priority queue for the telemetry module. It uses
 * linked list to solve the priority sorting, where the head of the list is
 * of a highest priority.
 *
 * /todo: Question - In header file, do we define every prototype that we need
 *        to use, or just the ones that are used outside, and the rest in the
 *        file locally?
 * /todo: Question - Do we define MAX_LENGTH for the queue? If so, is it max
 *        size of the data in it, or max number of nodes?
 * /todo: Define the data types for the telemetry.
 * /todo: Define priority values for the possible telemetry types.
 */

typedef struct node downlink_node;

static downlink_node* downlink_queue = NULL;

int init_downlink_queue( void ){
    return SUCCESS;
}

/*!
 * Node structure declaration.
 */
typedef struct node {
    int data;           // Data to be send as a telemetry.
    int priority;       // Lower values indicate higher priority
    struct node* next;  // Pointer to the node next on the list.

} downlink_node;

/*!
 * Function to create a new node
 *
 * @param d     Data to be sent.
 * @param p     Priority of the data.
 */
downlink_node* new_node(int d, int p)
{
    downlink_node* temp = (downlink_node*)malloc(sizeof(downlink_node));
    temp->data = d;
    temp->priority = p;
    temp->next = NULL;

    return temp;
}

/*!
 * Function to check if the list is empty
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      1 if the list is empty, 0 if not.
 */
 int is_empty( void ) {
    return (downlink_queue) == NULL;
 }
//int is_empty(downlink_node **head)
//{
//    return (*head) == NULL;
//}

/*!
 * Return the value at head of the list.
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      Data from the head node.
 */
int peek(downlink_node** head) {
    if(!is_empty()) {
        return (*head)->data;
    } else {
        return FAILURE;
    }
}

/*!
 * Removes the element with the highest priority form the list and
 * return it's data value.
 *
 * @param head  Pointer to the first node of the linked list.
 * @return      Data from the popped node.
 */
int pop(downlink_node** head)
{
    if(!is_empty()) {
        downlink_node *temp = *head;
        int data = (*head)->data;
        (*head) = (*head)->next;
        free(temp);
        return data;
    } else {
        return FAILURE;
    }
}

/*!
 * Function to push node to the list according to priority.
 *
 * @param head  Pointer to the first node of the linked list.
 * @param d     Data to be sent.
 * @param p     Priority of the data.
 */
void push(downlink_node** head, int d, int p)
{
    downlink_node* start = (*head);

    // Create new node
    downlink_node* temp = new_node(d, p);

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
               start->next->priority < p) {
            start = start->next;
        }
        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;
    }
}

/*!
 * Function used to put data into the queue.
 *
 * @param d     Data to be sent.
 * @param p     Priority of the data.
 * @return      0
 */
int send_telemetry_local(int d, int p){
    if(!is_empty()){
        push(&downlink_queue, d, p);
    } else {
        downlink_queue = new_node(d, p);
    }

    return SUCCESS;
}

int read_downlink_queue( void ) {
    return pop(&downlink_queue);
}