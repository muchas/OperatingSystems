#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include "utils.h"

void get_task(int *memory)
{
    int queue_start;
    time_t now;
    char *timestamp;

    queue_start = memory[START_INDEX];

    memory[START_INDEX] = (queue_start + 1) % ARRAY_SIZE;
    memory[SIZE_INDEX] -= 1;

    time(&now);
    timestamp = malloc(sizeof(char)*16);
    strftime(timestamp, 15, "%H:%M:%S", localtime(&now));

    if(memory[queue_start] % 2 == 0) {
        printf("(%d %s) Sprawdzilem liczbe %d na pozycji %d - parzysta. Pozostalo zadan oczekujacych: %d\n",
               getpid(), timestamp, memory[queue_start], queue_start, memory[SIZE_INDEX]);
    } else {
        printf("(%d %s) Sprawdzilem liczbe %d na pozycji %d - nieparzysta. Pozostalo zadan oczekujacych: %d\n",
               getpid(), timestamp, memory[queue_start], queue_start, memory[SIZE_INDEX]);
    }

    free(timestamp);
}


void consume(int *memory, int semaphore_id)
{
    while(1) {
        acquire_semaphore(semaphore_id);
        if(memory[SIZE_INDEX] > 0) {
            get_task(memory);
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
        perror("Creating semaphore");
        exit(EXIT_FAILURE);
    }

    printf("%d\n", memory[SIZE_INDEX]);

    consume(memory, semaphore_id);

    return EXIT_SUCCESS;
}