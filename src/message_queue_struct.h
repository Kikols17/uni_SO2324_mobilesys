#ifndef MESSAGE_QUEUE_STRUCT_H
#define MESSAGE_QUEUE_STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

typedef struct message_queue {
    char **queue;
    int size;
    int buf_size;
    pthread_mutex_t lock;
    int read_index;
    int write_index;
    int count;          // Number of messages in the queue
} message_queue;


message_queue *create_message_queue(int size, int buf_size);
void destroy_message_queue(message_queue *mq);


int write_message_queue(message_queue *mq, char *msg);
int read_message_queue(message_queue *mq, char *msg);

#endif