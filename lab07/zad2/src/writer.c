#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "utils.h"



void put_numbers(int *memory)
{
    char *timestamp;
    int i, count, number, index;
    time_t now;

    timestamp = malloc(sizeof(char)*16);

    count = rand() % ARRAY_SIZE;
    number = rand() % 15;

    for(i=0; i<count; i+=1) {
        index = rand() % ARRAY_SIZE;
        memory[index] = number + i;

        time(&now);
        strftime(timestamp, 15, "%H:%M:%S", localtime(&now));
        printf("(%d %s) Wpisalem liczbe %d na pozycje %d. Pozostalo %d zadan.\n", getpid(), timestamp, number + i, index, count-i-1);
    }

    free(timestamp);
}


void write_to_array(int *memory, sem_t *semaphore_fd)
{
    while(1) {
        acquire_semaphore(semaphore_fd);
        put_numbers(memory);
        release_semaphore(semaphore_fd);

        sleep(3);
    }
}


int main(int argc, char *argv[])
{
    int *memory, number;
    sem_t *semaphore_fd;

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <memory_name> <number>\n");
        exit(EXIT_FAILURE);
    }

    memory = get_shared_memory(argv[1], PROT_READ | PROT_WRITE);
    semaphore_fd = get_semaphore(WRITER_SEMAPHORE_NAME);

    write_to_array(memory, semaphore_fd);

    sem_close(semaphore_fd);

    return EXIT_SUCCESS;
}