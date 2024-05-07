#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "message_queue_struct.h"
#include "message_queue_struct.c"

message_queue *create_message_queue(int size, int buf_size);
void destroy_message_queue(message_queue *mq);
int write_message_queue(message_queue *mq, char *msg);
int read_message_queue(message_queue *mq, char *msg);

#define QUEUE_SIZE 10
#define BUF_SIZE 100

void *write_to_queue(void *arg) {
    message_queue *mq = (message_queue *)arg;
    char msg[BUF_SIZE];
    for (int i = 0; i < QUEUE_SIZE; i++) {
	sleep(rand()%5);
        sprintf(msg, "Message %d", i);
        while (write_message_queue(mq, msg) == 1); // Keep trying until message is written
	printf("Write: %s\n", msg);
    }
    return NULL;
}

void *read_from_queue(void *arg) {
    message_queue *mq = (message_queue *)arg;
    char msg[BUF_SIZE];
    for (int i = 0; i < QUEUE_SIZE; i++) {
	sleep(rand()%10);
        while (read_message_queue(mq, msg) == 1); // Keep trying until message is read
        printf("Read: %s\n", msg);
    }
    return NULL;
}

int main() {
    message_queue *mq = create_message_queue(QUEUE_SIZE, BUF_SIZE);
    pthread_t writer, reader;

    pthread_create(&writer, NULL, write_to_queue, mq);
    pthread_create(&reader, NULL, read_from_queue, mq);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    destroy_message_queue(mq);
    return 0;
}
