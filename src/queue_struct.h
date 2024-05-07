#ifndef MESSAGE_QUEUE_STRUCT_H
#define MESSAGE_QUEUE_STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

typedef struct queue {
    char **queue;
    int size;
    int buf_size;
    pthread_mutex_t lock;
    pthread_cond_t *cond;   // pointer to the condition variable that will be used
    int read_index;
    int write_index;
    int count;          // Number of messages in the queue
} queue;


queue *create_queue(int size, int buf_size, pthread_cond_t *cond);
void destroy_queue(queue *q);


int write_queue(queue *q, char *msg);
int read_queue(queue *q, char *msg);

#endif