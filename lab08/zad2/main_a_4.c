#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <signal.h>
#define RECORD_SIZE 1024
#define BUFFER_SIZE (RECORD_SIZE-sizeof(int))


typedef struct ThreadArgs {
    int signal;
} thread_args_t;



static int threads_number;
static pthread_t *thread_ids;
static pthread_key_t thread_buffer_key;


void *read_records(void *parameters)
{
    sigset_t set;
    thread_args_t *p;

    p = (thread_args_t *) parameters;

    sigemptyset(&set);
    sigaddset(&set, p->signal);

    if(sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
        perror("Error in sigprocmask func\n");
    }

    printf("TID: %ld PID: %d\n", syscall(SYS_gettid), (int) getpid());

    while(1) {}

    return NULL;
}


void run_threads(int number, int sig)
{

    int i;
    thread_args_t args;

    args.signal = sig;

    thread_ids = malloc(sizeof(pthread_t)*number);

    for(i=0; i<number; i+=1) {
        pthread_create(&thread_ids[i], NULL, read_records, &args);
    }

    sleep(5);

    if(pthread_kill(thread_ids[0], sig) < 0) {
        perror("Error in pthread kill \n");
    }

    printf("Signal %d sent!\n", sig);

    for(i=0; i<number; i+=1) {
        pthread_join(thread_ids[i], NULL); // parameters: thread_id, returned value
    }

    free(thread_ids);
}


int parse_int(char* arg)
{
    long num;
    char *error;

    num = strtol(arg, &error, 10);

    if(strlen(error)) {
        fprintf(stderr, "Argument %s is not a number.\n", arg);
        exit(EXIT_FAILURE);
    }

    if(num >= INT_MAX) {
        fprintf(stderr, "Argument %s has exceeded integer limit.\n", arg);
        exit(EXIT_FAILURE);
    }

    return (int)num;
}


int main(int argc, char* argv[])
{
    int records_number, fd, signal;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <threads_number> <signal>\n");
        exit(EXIT_FAILURE);
    }

    printf("Main process - PID: %d\n", (int) getpid());

    threads_number = parse_int(argv[1]);
    signal = parse_int(argv[2]);

    run_threads(threads_number, signal);

    return EXIT_SUCCESS;
}