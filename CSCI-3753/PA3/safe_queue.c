#include "safe_queue.h"

#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

struct SafeQueueImp {
    void** datas; // share data
    int front, rear; // the front, rear point.
    int cap; // the capacity.
    // Synchronization for not emtpy or not full status to queue.
    pthread_mutex_t* lock;
    pthread_cond_t* not_empty;
    pthread_cond_t* not_full;
};

SafeQueue createQueue(int cap) {
    SafeQueue sfq = malloc(sizeof(struct SafeQueueImp));
    sfq->cap = cap > 0 ? cap : 1024;
    sfq->datas = malloc(sfq->cap * sizeof(void*));
    sfq->front = sfq->rear = 0;
    sfq->lock = malloc(sizeof(pthread_mutex_t));
    sfq->not_empty = malloc(sizeof(pthread_cond_t));
    sfq->not_full = malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(sfq->lock, NULL);
    pthread_cond_init(sfq->not_empty, NULL);
    pthread_cond_init(sfq->not_full, NULL);
    return sfq;
}

void freeQueue(SafeQueue que) {
    free(que->datas);
    free(que->lock);
    free(que->not_empty);
    free(que->not_full);
    free(que);
}

void pushQueue(SafeQueue que, void* data) {
    pthread_mutex_lock(que->lock);
    while ((que->rear + 1) % que->cap == que->front) {
        pthread_cond_wait(que->not_full, que->lock);
    }
    que->datas[que->rear] = data;
    que->rear = (que->rear+1) % que->cap;
    pthread_mutex_unlock(que->lock);
    pthread_cond_broadcast(que->not_empty);
}

static struct timespec cal_timeout(size_t timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timespec abstime;
    abstime.tv_sec = now.tv_sec;
    abstime.tv_nsec = now.tv_usec * 1000 + timeout * 1000000;
    abstime.tv_sec += abstime.tv_nsec / 1000000000;
    abstime.tv_nsec %= 1000000000;
    return abstime;
}

void* popQueue(SafeQueue que, size_t timeout) {
    pthread_mutex_lock(que->lock);
    while (que->front == que->rear) {
        struct timespec abstime = cal_timeout(timeout);
        if (pthread_cond_timedwait(que->not_empty, que->lock, &abstime) == ETIMEDOUT) {
            pthread_mutex_unlock(que->lock);
            return NULL;
        }
    }
    void* ret = que->datas[que->front];
    que->front = (que->front + 1) % que->cap;
    pthread_mutex_unlock(que->lock);
    pthread_cond_broadcast(que->not_full);
    return ret;
}

int emptyQueue(SafeQueue que) {
    pthread_mutex_lock(que->lock);
    int ret = que->front == que->rear;
    pthread_mutex_unlock(que->lock);
    return ret;
}
