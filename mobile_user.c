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


#define BUF_SIZE 64

typedef struct Settings {
    int init_plafond;
    int max_request;
    int video_interval, music_interval, social_interval;
    int request_size;
} Settings;



int validate_settings();
int auth5g_register();
int auth5g_request(char *req_type);



// Hold all the settings for this mobile user
struct Settings settings;
int requests_left;



int main(int argc, char *argv[]) {

    if (argc!=7) {
        printf("!!!INCORRECT ARGUMENTS!!!\n-> %s {plafond inicial}\n{número máximo de pedidos de autorização}\n{intervalo VIDEO} {intervalo MUSIC} {intervalo SOCIAL}\n{dados a reservar}\n", argv[0]);
        return 1;
    }

    settings.init_plafond = atoi(argv[1]);
    settings.max_request = atoi(argv[2]);
    requests_left = settings.max_request;

    settings.video_interval = atoi(argv[3]);
    settings.music_interval = atoi(argv[4]);
    settings.social_interval = atoi(argv[5]);

    settings.request_size = atoi(argv[6]);


    printf("Creating user:\n\tinitial plafond->%d\n\tmax requests->%d\n\tvideo interval->%d\n\tmusic interval->%d\n\tsocial interval->%d\n\trequest size->%d\n", settings.init_plafond, settings.max_request, settings.video_interval, settings.music_interval, settings.social_interval, settings.request_size);

    if ( validate_settings() != 0 ) {
        return 1;
    }


    auth5g_register();



    return 0;
}



int validate_settings() {
    /* Returns 0 if "settings" is up to code;
     * Returns 1 if "settings" is wrongly configured, and prints to stderr what is wrong
     */
    int flag = 0;
    if (settings.init_plafond <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {plafond inicial} must be >= 1\n");
        flag = 1;
    }
    if (settings.max_request <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {número máximo de pedidos de autorização} must be >= 1\n");
        flag = 1;
    }
    if (settings.video_interval <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {intervalo VIDEO} must be >= 1\n");
        flag = 1;
    }
    if (settings.music_interval <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {intervalo MUSIC} must be >= 1\n");
        flag = 1;
    }
    if (settings.social_interval <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {intervalo SOCIAL} must be >= 1\n");
        flag = 1;
    }
    if (settings.request_size <= 0) {
        fprintf(stderr, "[ERROR_ARGS]: {dados a reservar} must be >= 1\n");
        flag = 1;
    }
    return flag;
}



int auth5g_register() {
    char buff_out[BUF_SIZE];
    sprintf(buff_out, "%d#%d", getpid(), settings.init_plafond);
    fprintf(stdout, "[REGISTER]: \"%s\"\n", buff_out);
    return 0;
}


int auth5g_request(char *req_type) {
    char buff_out[BUF_SIZE];
    if (requests_left==0) {
        return 1;
    }
    sprintf(buff_out, "%d#%s#%d", getpid(), req_type, settings.request_size);
    fprintf(stdout, "[REQUEST]: \"%s\"\n", buff_out);
    requests_left--;
    return 0;
}