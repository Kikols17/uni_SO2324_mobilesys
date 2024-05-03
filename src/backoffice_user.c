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


#define BACKEND_PIPE "/tmp/backend_pipe"

#define BUF_SIZE 1024
#define MESSAGE_QUEUE 1234


int auth5g_request(char *req_type);


int backendpipe_fd;


int main(int argc, char *argv[]) {
    char buff_out[BUF_SIZE];            // Store messages to write to pipe

    if (argc!=1) {
        printf("!!!INCORRECT ARGUMENTS!!!\n-> %s\n", argv[0]);
    }


    if ((backendpipe_fd = open(BACKEND_PIPE, O_WRONLY)) < 0) {
        fprintf(stderr, "[ERROR]: Cannot open pipe for writing");
        exit(0);
    }

    char option[2];
    while (1) {
        fprintf(stdout, "\n0: data_stats - apresenta estatísticas referentes aos consumos dos dados nos vários serviços:\n"
        "total de dados reservados e número de pedidos de renovação de autorização;\n"
        "1: reset - limpa as estatísticas relacionadas calculadas até ao momento pelo sistema.\n");
        fgets(option, 2, stdin);
        if (option[0]=='0') {
            auth5g_request("data_stats");
        } else if (option[0]=='1') {
            auth5g_request("reset");
        } else {
            fprintf(stderr, "[ERROR]: Invalid option\n");
        }
    } 

    sprintf(buff_out, "%d#%s", getpid(), "data_stats");
    printf("MAKE REQUEST: \"%s\"\n", buff_out);
    write(backendpipe_fd, &buff_out, sizeof(buff_out));

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