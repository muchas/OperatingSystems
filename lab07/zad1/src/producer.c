#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include "utils.h"


void create_task(int *memory)
{
    char *timestamp;
    int number, queue_end;
    time_t now;


    queue_end = memory[END_INDEX];

    number = rand() % 1000;

    memory[queue_end] = number;
    memory[END_INDEX] = (queue_end + 1) % ARRAY_SIZE;
    memory[SIZE_INDEX] += 1;

    time(&now);
    timestamp = malloc(sizeof(char)*16);
    strftime(timestamp, 15, "%H:%M:%S", localtime(&now));

    printf("(%d %s) Dodalem %d na pozycje %d. Liczba zadan oczekujacych: %d\n", getpid(), timestamp, number, queue_end, memory[SIZE_INDEX]);

    free(timestamp);
}


void produce(int *memory, int semaphore_id)
{
    while(1) {
        acquire_semaphore(semaphore_id);
        if(memory[SIZE_INDEX] < ARRAY_SIZE) {
            create_task(memory);
        }
        release_semaphore(semaphore_id);
        sleep(1);
    }
}


int main(int argc, char *argv[])
{
    int *memory, semaphore_id;
    key_t key;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <pathname> <project_id>\n");
        exit(EXIT_FAILURE);
    }


    key = get_key(argv[1], parse_int(argv[2]));
    memory = get_shared_memory(key, 0);

    if((semaphore_id = semget(key, SEMAPHORE_NUM, 0)) == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    printf("%d\n", memory[END_INDEX]);

    produce(memory, semaphore_id);

    return EXIT_SUCCESS;
}