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
    int fd;
    size_t records_number;
    char *phrase;
} thread_args_t;


typedef struct RecordData {
    int id;
    char text[BUFFER_SIZE];
} record_data_t;


typedef union Record {
    char bytes[RECORD_SIZE];
    record_data_t data;
} record_t;


static int threads_number;
static pthread_t *thread_ids;
static pthread_key_t thread_buffer_key;


void cancel_threads(pthread_t caller_id)
{
    int i;
    for(i=0; i<threads_number; i+=1) {
        if(!pthread_equal(caller_id, thread_ids[i])) {
            pthread_cancel(thread_ids[i]);
        }
    }
}

void close_thread_buffer(void *thread_buffer)
{
    free((char *) thread_buffer);
}


void notify_about_exit(void *arg)
{
    printf("TID: %ld exited!\n", syscall(SYS_gettid));
}


void *read_records(void *parameters)
{
    printf("TID: %ld PID: %d\n", syscall(SYS_gettid), (int) getpid());

    while(1) {}

    return NULL;
}


void run_threads(int number, int signal)
{

    int i;
    thread_args_t args;

    thread_ids = malloc(sizeof(pthread_t)*number);

    pthread_key_create(&thread_buffer_key, close_thread_buffer);

    for(i=0; i<number; i+=1) {
        pthread_create(&thread_ids[i], NULL, read_records, NULL);
    }


    sleep(5);
    kill(getpid(), signal);

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