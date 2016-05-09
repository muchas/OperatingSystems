#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "utils.h"


void find_number(int number, int *memory)
{
    char *timestamp;
    int i, count;
    time_t now;

    count = 0;
    for(i=0; i<ARRAY_SIZE; i+=1) {
        if(memory[i] == number) {
            count++;
        }
    }

    time(&now);
    timestamp = malloc(sizeof(char)*16);
    strftime(timestamp, 15, "%H:%M:%S", localtime(&now));

    printf("(%d %s) Znalazlem %d liczb o wartosci %d\n", getpid(), timestamp, count, number);

    free(timestamp);
}


void read_array(int number, int *memory, sem_t* mutex_fd, sem_t* writer_semaphore_fd)
{
    while(1) {
        acquire_semaphore(mutex_fd);
        memory[READERS_COUNT_INDEX] += 1;
        if(memory[READERS_COUNT_INDEX] == 1) {
            acquire_semaphore(writer_semaphore_fd);
        }
        release_semaphore(mutex_fd);

        find_number(number, memory);

        acquire_semaphore(mutex_fd);
        memory[READERS_COUNT_INDEX] -= 1;
        if(memory[READERS_COUNT_INDEX] == 0) {
            release_semaphore(writer_semaphore_fd);
        }
        release_semaphore(mutex_fd);

        sleep(1);
    }
}


int main(int argc, char *argv[])
{
    int *memory, number;
    sem_t *mutex_fd, *writer_semaphore_fd;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <memory_name> <number>\n");
        exit(EXIT_FAILURE);
    }

    number = parse_int(argv[2]);
    memory = get_shared_memory(argv[1], PROT_READ | PROT_WRITE);
    mutex_fd = get_semaphore(READER_MUTEX_NAME);
    writer_semaphore_fd = get_semaphore(WRITER_SEMAPHORE_NAME);

    read_array(number, memory, mutex_fd, writer_semaphore_fd);

    sem_close(mutex_fd);
    sem_close(writer_semaphore_fd);

    return EXIT_SUCCESS;
}