/* 
 * Versão de 1ªMeta da disciplina Sistemas Operativos
 * 
 * TRABALHO REALIZADO POR:
 *      Francisco Amado Lapa Marques Silva - 2022213583
 *      Miguel Moital Rodrigues Cabral Martins - 2022213951
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/select.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>


#define LOG_FILE "log.txt"

#define MOBILE_PIPE "/tmp/mobile_pipe"
#define BACKEND_PIPE "/tmp/backend_pipe"

#define BUF_SIZE 1024


// Verify that "settings"' values are valid
int validate_settings();

// Write to log, and read from .config file
int append_logfile(char *log_info);
int read_configfile(char *configfile);

// Handle SIGINT signal (^C), closing all processes cleanly, and freeing shared memory
void sigint_handler();

// Create shared memory
int create_sharedmem();

/* Creates child proccesses parallel to the parent proccess
 * parallel_AuthorizationRequestManager creates ARM
 * parallel_AuthorizationEngine creates AE
 * parallel_MonitorEngine creates ME
 */
int parallel_AuthorizationRequestManager();
int parallel_AuthorizationEngine(int n);
int parallel_MonitorEngine();

/* Authorization_Request_Manager's threads.
 * RECEIVER: Used to recieve requests from mobile clients
 * SENDER: Used to send to Authorization_Engine's
 */
void *receiver_ARM( void *arg );
void *sender_ARM( void *arg );
int check_message_queue(char *message);



typedef struct Settings {
    int MOBILE_USERS;       // número de Mobile Users que serão suportados pelo simulador
    int QUEUE_POS;          // número de slots nas filas que são utilizadas para armazenar os pedidos de autorização e os comandos dos utilizadores (>=0)
    int AUTH_SERVERS;       // número de Authorization Engines que devem ser lançados inicialmente (>=1)
    int AUTH_PROC_TIME;     // período (em ms) que o Authorization Engine demora para processar os pedidos

    int MAX_VIDEO_WAIT;     // tempo máximo (em ms) que os pedidos de autorização do serviço de vídeo podem aguardar para serem executados (>=1)
    int MAX_OTHERS_WAIT;    // tempo máximo (em ms) que os pedidos de autorização dos serviços de música e de redes sociais, bem como os comandos podem aguardar para serem executados (>=1)
} Settings;

typedef struct User_data {
    int id;
    int plafond_left;
} User_data;


sem_t *log_sem;
struct Settings settings;
int pid;
int system_manager_pid;

int shmid;
User_data *user_array;

int main(int argc, char *argv[]) {
    system_manager_pid = getpid();

    signal(SIGINT, sigint_handler);       // redirect ^C (ctrl+c) command
    /* Verify correct amount of arguments */
    if (argc!=2) {
        fprintf(stderr, "!!!INCORRECT ARGUMENTS!!!\n-> %s {config-file}\n", argv[0]);
        exit(1);
    }

    
    /* Read config file, and set "settings" */
    if (read_configfile(argv[1])!=0) {
        exit(1);
    }

    /* Verify that settings on config file are valid */
    if ( validate_settings()!=0 ) {
        exit(1);
    }

    /* Create semaphore for log file */
    log_sem = sem_open("log_sem", O_CREAT, 0777, 1);
    if ( log_sem==SEM_FAILED ) {
        fprintf(stderr, "ERROR CAN'T CREATE SEMAPHORE\n");
        sigint_handler();
    }

    /* Create shared memory */
    if ( create_sharedmem()!=0 ) {
        sigint_handler();
    }

    append_logfile("5G_AUTH_PLATFORM SIMULATOR STARTING");
    append_logfile("PROCESS SYSTEM_MANAGER CREATED");

    // Create child proccesses of the system
    parallel_AuthorizationRequestManager();
    parallel_MonitorEngine();

    while (1) {}        // mantain open to manage the shared memory and semaphores
    return 0;
}



int validate_settings() {
    /* Returns 0 if "settings" is up to code;
     * Returns 1 if "settings" is wrongly configured, and prints to stderr what is wrong
     */
    int flag = 0;
    if (settings.MOBILE_USERS <= 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {MOBILE_USERS} must be >= 1\n");
        flag = 1;
    }
    if (settings.QUEUE_POS <= 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {QUEUE_POS} must be >= 1\n");
        flag = 1;
    }
    if (settings.AUTH_SERVERS <= 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {AUTH_SERVERS} must be >= 1\n");
        flag = 1;
    }
    if (settings.AUTH_PROC_TIME < 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {AUTH_PROC_TIME} must be >= 0\n");
        flag = 1;
    }
    if (settings.MAX_VIDEO_WAIT <= 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {MAX_VIDEO_WAIT} must be >= 1\n");
        flag = 1;
    }
    if (settings.MAX_OTHERS_WAIT <= 0) {
        fprintf(stderr, "[ERROR_CONFIG]: {MAX_OTHERS_WAIT} must be >= 1\n");
        flag = 1;
    }
    return flag;
}




void sigint_handler() {
    if ( system_manager_pid==getpid() ) {
        /* If is father proccess (SYSTEM_MANAGER) */
        append_logfile("SIGNAL SIGINT RECEIVED");
        append_logfile("5G_AUTH_PLATFORM SIMULATOR WAITING FOR LAST TASKS TO FINISH");
        wait(NULL);
        append_logfile("5G_AUTH_PLATFORM SIMULATOR CLOSING\n+----------------------------------------------------------------------+");
        sem_close(log_sem);         // }
        sem_unlink("log_sem");      // } unlink and close log_sem
        shmdt(user_array);              // }
        shmctl(shmid, IPC_RMID, 0);     // } free shared memory
    } else {
        /* If is child proccess (ARM/AE/ME), send SIGINT to father */
        sleep(0.5);
        append_logfile("CHILD PROCESS TERMINATED");
        kill(system_manager_pid, SIGINT);
    }
        
    exit(0);
}

int create_sharedmem() {
    /* Create sharedmemory (user_array) */
    shmid = shmget(IPC_PRIVATE, sizeof(User_data) * settings.MOBILE_USERS, IPC_CREAT | 0777);
    if (shmid == -1) {
        fprintf(stderr, "[ERROR] Could not create shared memory\n");
        return 1;
    }
    user_array = (User_data*) shmat(shmid, NULL, 0);
    if (user_array == (void*)-1) {
        fprintf(stderr, "[ERROR] Could not assign shared memory\n");
        return 1;
    }
    return 0;
}



int append_logfile(char *log_info) {
    sem_wait(log_sem);
    FILE *log = fopen(LOG_FILE, "a");
    if (log==NULL) {
        // Could not open file
        fprintf(stderr, "[ERROR] Could not open file \"%s\".\n", LOG_FILE);
        // Don't log error, obviously
        return 1;
    }

    time_t posix_time;
    struct tm* time_struct;
    char time_buffer[80];

    time(&posix_time);      // Get time since epoch
    time_struct = localtime(&posix_time);

    strftime(time_buffer, 80, "%X", time_struct);       // write time hour:min:sec
    fprintf(log,    "%s %s\n", time_buffer, log_info);      // } Write log info to log.txt
    fprintf(stdout, "%s %s\n", time_buffer, log_info);      // } and to terminal

    fclose(log);
    sem_post(log_sem);
    return 0;
}

int read_configfile(char *configfile) {
    /* Read from filepath "configfile", and write to global Settings "settings" */
    FILE *file = fopen(configfile, "r");
    if (file==NULL) {
        // could not open file
        fprintf(stderr, "[ERROR] Could not open config file \"%s\".\n", configfile);
        return 1;
    }

    if ( fscanf(file, "%d\n%d\n%d\n%d\n%d\n%d", &settings.MOBILE_USERS, &settings.QUEUE_POS, &settings.AUTH_SERVERS, &settings.AUTH_PROC_TIME, &settings.MAX_VIDEO_WAIT, &settings.MAX_OTHERS_WAIT)!=6 ) {
        // if fscanf does not read 6 numbers, fail, incorrect format
        fprintf(stderr, "[ERROR] File \"%s\" is not formated correctly.\n", configfile);
        return 1;
    }

    // DEBUG show settings
    fprintf( stdout, "[DEBUG] Settings:\n\tMOBILE_USERS->%d\n\tQUEUE_POS->%d\n\tAUTH_SERVERS->%d\n\tAUTH_PROC_TIME->%d\n\tMAX_VIDEO_WAIT->%d\n\tMAX_OTHERS_WAIT->%d\n", settings.MOBILE_USERS, settings.QUEUE_POS, settings.AUTH_SERVERS, settings.AUTH_PROC_TIME, settings.MAX_VIDEO_WAIT, settings.MAX_OTHERS_WAIT );

    fclose(file);
    return 0;
}



int parallel_AuthorizationRequestManager() {
    /* Creates a new process that will have the role of AUTHORIZATION_REQUEST_MANAGER */
    pid = fork();
    if (pid<0) {
        // fork failed;
        append_logfile("[ERROR] FAILED TO CREATE PROCESS AUTHORIZATION_REQUEST_MANAGER");
        return 1;
    } else if (pid!=0) {
        // if pid=0, parent process, return
        return 0;
    }

    /* if pid=0, child process, now authorization request manager */

    pthread_t reciever, sender;
    append_logfile("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
    
    int namedpipes[2];      // 0 for BACKENDPIPE; 1 for MOBILEPIPE;
    if ( (mkfifo(BACKEND_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!=EEXIST) ) {
        // Creates the BACKEND named pipe if it doesn't exist yet
        perror("Cannot create pipe: ");
        exit(1);
    } else if ((namedpipes[0] = open(BACKEND_PIPE, O_RDWR)) < 0) {
        // Opens BACKEND named pipe for reading
        perror("Cannot open pipe for reading: ");
        exit(1);
    } else {
        append_logfile("BACKEND PIPE CREATED");
    }

    if ( (mkfifo(MOBILE_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!=EEXIST) ) {
        // Creates the BACKEND named pipe if it doesn't exist yet
        perror("Cannot create pipe: ");
        exit(1);
    } else if ((namedpipes[1] = open(MOBILE_PIPE, O_RDWR)) < 0) {
        // Opens BACKEND named pipe for reading
        perror("Cannot open pipe for reading: ");
        exit(1);
    } else {
        append_logfile("MOBILEUSER PIPE CREATED");
    }

    // TODO create message queues
    

    

    for (int i=0; i<settings.AUTH_SERVERS; i++) {
        parallel_AuthorizationEngine(i+1);
    }

    pthread_create(&reciever, NULL, receiver_ARM, &namedpipes);
    pthread_create(&sender, NULL, sender_ARM, NULL);
    pthread_join(reciever, NULL);
    pthread_join(sender, NULL);

    exit(0);
}

int parallel_AuthorizationEngine(int n) {
    /* Creates a new process that will have the role of AUTHORIZATION_ENGINE */
    pid = fork();
    if (pid<0) {
        // fork failed;
        append_logfile("[ERROR] FAILED TO CREATE PROCESS AUTHORIZATION_ENGINE");
        return 1;
    } else if (pid!=0) {
        // if pid=0, parent process, return
        return 0;
    }


    // if pid=0, child process, now authorization engine
    int id = n;
    char message[BUF_SIZE];
    sprintf(message, "PROCESS AUTHORIZATION_ENGINE %d CREATED", id);
    append_logfile(message);

    while (1) {}        // TODO[META1] do AUTHORIZATION_ENGINE
    exit(0);
}

int parallel_MonitorEngine() {
    /* Creates a new process that will have the role of MONITOR_ENGINE */
    pid = fork();
    if (pid<0) {
        // fork failed;
        append_logfile("[ERROR] FAILED TO CREATE PROCESS MONITOR_ENGINE");
        return 1;
    } else if (pid!=0) {
        // if pid=0, parent process, return
        return 0;
    }


    // if pid=0, child process, now monitor engine
    append_logfile("PROCESS MONITOR_ENGINE CREATED");
    while (1) {}        // TODO[META1] do MONITOR_ENGINE
    exit(0);
}



void *receiver_ARM( void *arg ) {
    /* Reciever (AUTHORIZATION_REQUEST_MANAGER) */
    char inbuffer[BUF_SIZE];
    int mobile_pipe_fd, backend_pipe_fd;

    append_logfile("THREAD RECEIVER CREATED");

    backend_pipe_fd = ((int*) arg)[0];
    mobile_pipe_fd = ((int*) arg)[1];

    fd_set readfds;
    while (1) {
        // Set FD_SET to read from backend_pipe_fd and mobile_pipe_fd
        FD_ZERO(&readfds);
        FD_SET(backend_pipe_fd, &readfds);
        FD_SET(mobile_pipe_fd, &readfds);

        // Wait until something is written to one of the pipes
        if ( select(mobile_pipe_fd+1, &readfds, NULL, NULL, NULL)>0 ) {
            if (FD_ISSET(backend_pipe_fd, &readfds)) {
                // If is written to backend_pipe_fd
                if (read(backend_pipe_fd, &inbuffer, sizeof(inbuffer))!=EOF) {
                    printf("[RECEIVED] BACKEND -> %s\n", inbuffer);
                }
            }
            if (FD_ISSET(mobile_pipe_fd, &readfds)) {
                // If is written to mobile_pipe_fd
                if (read(mobile_pipe_fd, &inbuffer, sizeof(inbuffer))!=EOF) {
                    printf("[RECEIVED] MOBILE -> %s\n", inbuffer);
                }
            }
        }
    }
    return NULL;
}

void *sender_ARM( void *arg ) {
    /* Sender (AUTHORIZATION_REQUEST_MANAGER) */
    append_logfile("THREAD SENDER CREATED");
    while (1) {}        // TODO[META1] make sender
    return NULL;
}

//Funcao para verificar se a mensagem vai para a message queue de video ou de outros
int check_message_queue(char *message) {
    strtok(message, " ");
    char *token = strtok(NULL, " ");
    if (strcmp(token, "VIDEO") == 0) {
        //Mensagem vai para a message queue de video
        return 1;
    } else {
        //Mensagem vai para a message queue de outros
        return 0;
    }
    return -1;
}
