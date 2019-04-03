/*
 * Faraz Heravi
 * queue.c
 */

#include <stdlib.h>
#include "queue.h"

// initialize queue
queue *createQueue() {
    queue *new = (queue *)malloc(sizeof(queue));
    new->head = NULL;
    new->tail = NULL;
    new->size = 0;
    return new;
}

// initialize node
node *initNode(void *data) {
    node *new = (node *)malloc(sizeof(node));
    new->next = NULL;
    new->data = data;
    return new;
}

// add to the queue
void enqueue(queue *queue, void *data) {
    node *n = initNode(data);
    if (queue->size == 0) {
        queue->head = n;
        queue->tail = n;
    } else {
        queue->tail->next = n;
        queue->tail = n;
    }
    queue->size += 1;
}

// remove from queue
void *dequeue(queue *queue) {
    if (queue->size == 0) {
        return NULL;
    }
    node *pop = queue->head;
    queue->head = pop->next;
    void *popped = pop->data;
    free(pop);
    queue->size -= 1;
    if (queue->size == 0) {
        queue->head = NULL;
        queue->tail = NULL;
    }
    return popped;
}

// check if queue's empty
int empty(queue *q) {
    return q->size == 0;
}
