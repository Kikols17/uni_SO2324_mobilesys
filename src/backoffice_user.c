/* 
 * Versão de 1ªMeta da disciplina Sistemas Operativos
 * 
 * TRABALHO REALIZADO POR:
 *      Francisco Amado Lapa Marques Silva - 2022213583
 *      Miguel Moital Rodrigues Cabral Martins - 2022213951
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>


#define BACKEND_PIPE "/tmp/backend_pipe"

#define BUF_SIZE 1024
#define MESSAGE_QUEUE 1234


int auth5g_request(char *req_type);

void *backend_response();
void *user_input();


int backendpipe_fd;
int message_queue_id;


int main(int argc, char *argv[]) {
    if (argc!=1) {
        printf("!!!INCORRECT ARGUMENTS!!!\n-> %s\n", argv[0]);
    }

    if ((backendpipe_fd = open(BACKEND_PIPE, O_WRONLY)) < 0) {
        fprintf(stderr, "[ERROR]: Cannot open pipe for writing");
        exit(0);
    }

    pthread_t user_input_thread, backend_response_thread;
    pthread_create(&user_input_thread, NULL, user_input, NULL);
    pthread_create(&backend_response_thread, NULL, backend_response, NULL);

    pthread_join(user_input_thread, NULL);
    msgctl(message_queue_id, IPC_RMID, NULL);
    pthread_join(backend_response_thread, NULL);

    return 1;
}


int auth5g_request(char *req_type) {
    /* Make request of type "req_type" */
    char buff_out[BUF_SIZE];            // Store messages to write to pipe

    sprintf(buff_out, "%d#%s", getpid(), req_type);
    printf("MAKE REQUEST: \"%s\"\n", buff_out);
    if (write(backendpipe_fd, &buff_out, sizeof(buff_out))==0 ) {
        fprintf(stderr, "[ERROR]: Cannot write to pipe");
        return 0;
    }

    return 1;
}


void *user_input() {
    /* Thread that handles the user input, and writes to pipe */
    char option[2];
    while (1) {
        /*
        fprintf(stdout, "\n0: data_stats - apresenta estatísticas referentes aos consumos dos dados nos vários serviços:\n"
        "total de dados reservados e número de pedidos de renovação de autorização;\n"
        "1: reset - limpa as estatísticas relacionadas calculadas até ao momento pelo sistema.\n"
        "2: exit - termina a execução do programa.\n");
        */
        fgets(option, 2, stdin);
        if (option[0]=='0') {
            auth5g_request("data_stats");
        } else if (option[0]=='1') {
            auth5g_request("reset");
        } else if (option[0]=='2') {
            pthread_exit(NULL);
        } else {
            fprintf(stderr, "[ERROR]: Invalid option\n");
        }
    }
    return NULL;
}

void *backend_response() {
    /* Thread that reads the message queue */
    char buff_in[BUF_SIZE];             // Store messages read from pipe

    if ( (message_queue_id = msgget(MESSAGE_QUEUE, 0666)) < 0 ) {
        fprintf(stderr, "[ERROR]: Cannot open message queue");
        exit(0);
    }
    while (1) {
        if (msgrcv(message_queue_id, &buff_in, sizeof(buff_in), getpid(), 0) < 0) {
            fprintf(stderr, "[ERROR]: Cannot read from message queue");
            exit(0);
        }
        printf("RECEIVED: \"%s\"\n", buff_in);
    }
    return NULL;
}