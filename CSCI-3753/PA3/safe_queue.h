/*
 * The thread safe Producer-consumer queue
 */
#include <stdio.h>
typedef struct SafeQueueImp* SafeQueue;

SafeQueue createQueue(int cap);
void freeQueue(SafeQueue que);
/*
 * Push the element, blocking until the queue not full.
 */
void pushQueue(SafeQueue que, void* data);
/*
 * Pop and return a element, blocking until the queue not empty or timeout.
 */
void* popQueue(SafeQueue que, size_t timeout);
/*
 * Return 1 if the queue is empty.
 */
int emptyQueue(SafeQueue que);
