#ifndef MESSAGE_QUEUE_STRUCT_C
#define MESSAGE_QUEUE_STRUCT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "message_queue_struct.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif




message_queue *create_message_queue(int size, int buf_size) {
    /* Creates a message queue on heap */
    message_queue *new_mq = (message_queue *)malloc(sizeof(message_queue));
    new_mq->queue = (char **)malloc(sizeof(char*) * size);
    for (int i = 0; i < size; i++) {
        new_mq->queue[i] = (char *)malloc(sizeof(char) * buf_size);
    }
    new_mq->size = size;
    new_mq->buf_size = buf_size;
    pthread_mutex_init(&new_mq->lock, NULL);
    new_mq->read_index = 0;
    new_mq->write_index = 0;
    new_mq->count = 0;
    return new_mq;
}

void destroy_message_queue(message_queue *mq) {
    /* Frees the message queue */
    for (int i = 0; i < mq->size; i++) {
        free(mq->queue[i]);
    }
    free(mq->queue);
    pthread_mutex_destroy(&mq->lock);
    free(mq);
}


int write_message_queue(message_queue *mq, char *msg) {
    /* Writes a message to the message queue
     * returns 0 if successful, 1 if queue is full
     */
    pthread_mutex_lock(&mq->lock);
    if (mq->count == mq->size) {
        pthread_mutex_unlock(&mq->lock);
        return 1;
    }
    strcpy(mq->queue[mq->write_index], msg);
    mq->write_index = (mq->write_index + 1) % mq->size;
    mq->count++;
    pthread_mutex_unlock(&mq->lock);
    return 0;
}

int read_message_queue(message_queue *mq, char *msg) {
    /* Reads a message from the message queue
     * returns 0 if successful, 1 if queue is empty (should never ask to read from an empty queue, but anyway)
     * copies the message to "msg" buffer
     */
    pthread_mutex_lock(&mq->lock);
    if (mq->count == 0) {
        pthread_mutex_unlock(&mq->lock);
        return 1;
    }
    strcpy(msg, mq->queue[mq->read_index]);
    mq->read_index = (mq->read_index + 1) % mq->size;
    mq->count--;
    pthread_mutex_unlock(&mq->lock);
    return 0;
}

#endif