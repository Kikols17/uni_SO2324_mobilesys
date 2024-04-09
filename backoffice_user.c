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


#define BUF_SIZE 1024

#define BACKEND_PIPE "/tmp/backend_pipe"


int main() {
    char buff_out[BUF_SIZE];            // Store messages to write to pipe


    int backendpipe_fd;
    if ((backendpipe_fd = open(BACKEND_PIPE, O_WRONLY)) < 0) {
        perror("[ERROR]: Cannot open pipe for writing");
        exit(0);
    }

    sprintf(buff_out, "%d#%s", getpid(), "data_stats");
    printf("MAKE REQUEST: \"%s\"\n", buff_out);
    write(backendpipe_fd, &buff_out, sizeof(buff_out));
    return 1;
}