/*
 * Faraz Heravi
 * queue.h
 */

#ifndef SPELLCHECKER_QUEUE_H
#define SPELLCHECKER_QUEUE_H

typedef struct node {
    void *data;
    struct node *next;
} node;

typedef struct queue {
    node *head;
    node *tail;
    int size;
} queue;

queue *createQueue();
node *initNode(void *);
void enqueue(queue *, void *);
void *dequeue(queue *);
int empty(queue *);

#endif //SPELLCHECKER_QUEUE_H
