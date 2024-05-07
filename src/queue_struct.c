#ifndef MESSAGE_QUEUE_STRUCT_C
#define MESSAGE_QUEUE_STRUCT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "queue_struct.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif




queue *create_queue(int size, int buf_size, pthread_cond_t *cond) {
    /* Creates a queue on heap */
    queue *new_q = (queue *)malloc(sizeof(queue));
    new_q->queue = (char **)malloc(sizeof(char*) * size);
    for (int i = 0; i < size; i++) {
        new_q->queue[i] = (char *)malloc(sizeof(char) * buf_size);
    }
    new_q->size = size;
    new_q->buf_size = buf_size;
    pthread_mutex_init(&new_q->lock, NULL);
    new_q->cond = cond;
    new_q->read_index = 0;
    new_q->write_index = 0;
    new_q->count = 0;
    return new_q;
}

void destroy_queue(queue *q) {
    /* Frees the queue */
    for (int i = 0; i < q->size; i++) {
        free(q->queue[i]);
    }
    free(q->queue);
    pthread_mutex_destroy(&q->lock);
    free(q);
}


int write_queue(queue *q, char *msg) {
    /* Writes a message to the queue
     * returns 0 if successful, 1 if queue is full
     */
    pthread_mutex_lock(&q->lock);
    if (q->count == q->size) {
        pthread_mutex_unlock(&q->lock);
        return 1;
    }
    strcpy(q->queue[q->write_index], msg);
    q->write_index = (q->write_index + 1) % q->size;
    q->count++;
    pthread_cond_signal(q->cond);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

int read_queue(queue *q, char *msg) {
    /* Reads a message from the queue
     * returns 0 if successful, 1 if queue is empty (should never ask to read from an empty queue, but anyway)
     * copies the message to "msg" buffer
     */
    pthread_mutex_lock(&q->lock);
    if (q->count == 0) {
        pthread_mutex_unlock(&q->lock);
        return 1;
    }
    strcpy(msg, q->queue[q->read_index]);
    q->read_index = (q->read_index + 1) % q->size;
    q->count--;
    pthread_mutex_unlock(&q->lock);
    return 0;
}

#endif