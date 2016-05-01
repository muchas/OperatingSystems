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


int find_record(size_t read_records_count, char *phrase)
{
    int i, j;
    char *buffer;
    record_t record;
    buffer = (char *) pthread_getspecific(thread_buffer_key);

    for(i=0; i<read_records_count; i+=1) {
        memcpy(record.bytes, &buffer[i*RECORD_SIZE], RECORD_SIZE);

        if (strstr(record.data.text, phrase) != NULL) {
            return record.data.id;
        }
    }

    return -1;
}


void *read_records(void *parameters)
{
    int i, j, id;
    thread_args_t *p;
    char *buffer;
    size_t read_bytes, buffer_size;

    p = (thread_args_t *) parameters;
    buffer_size = RECORD_SIZE*p->records_number;
    buffer = (char *) malloc(sizeof(char)*buffer_size);

    pthread_setspecific(thread_buffer_key, buffer);

    while((read_bytes = read(p->fd, buffer, buffer_size)) > 0) {

        pthread_testcancel();

        if((id = find_record(read_bytes/RECORD_SIZE, p->phrase)) != -1) {
            printf("TID: %ld Record id: %d\n", syscall(SYS_gettid), id);
            cancel_threads(pthread_self());
            break;
        }
    }

    if(read_bytes == -1) {
        perror("Could not read the file");
    }

    return NULL;
}


void run_threads(int number, int fd, int records_number, char *phrase)
{
    int i;
    thread_args_t args;

    thread_ids = malloc(sizeof(pthread_t)*number);

    args.fd = fd;
    args.records_number = records_number;
    args.phrase = phrase;

    pthread_key_create(&thread_buffer_key, close_thread_buffer);

    for(i=0; i<number; i+=1) {
        pthread_create(&thread_ids[i], NULL, read_records, &args);
    }

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
    int records_number, fd;

    if(argc != 5) {
        printf("Invalid number of arguments. Usage: <threads_number> <filename> <records_number> <phrase>\n");
        exit(EXIT_FAILURE);
    }

    threads_number = parse_int(argv[1]);
    records_number = parse_int(argv[3]);

    if((fd = open(argv[2], O_RDONLY)) == -1) {
        perror("Invalid pathname");
        return EXIT_FAILURE;
    }

    run_threads(threads_number, fd, records_number, argv[4]);

    close(fd);

    return EXIT_SUCCESS;
}