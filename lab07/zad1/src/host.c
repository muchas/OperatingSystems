#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include "utils.h"



int main(int argc, char* argv[])
{
    int *memory, semaphore_id;
    char cmd;
    key_t key;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <pathname> <project_id>\n");
        exit(EXIT_FAILURE);
    }

    key = get_key(argv[1], parse_int(argv[2]));
    printf("%d\n", key);
    memory = get_shared_memory(key, 0666 | IPC_CREAT);

    if((semaphore_id = semget(key, SEMAPHORE_NUM, 0666 | IPC_CREAT)) == -1) {
        perror("Creating semaphore");
        exit(EXIT_FAILURE);
    }

    memory[SIZE_INDEX] = 0;
    memory[START_INDEX] = 0;
    memory[END_INDEX] = 0;

    cmd = '0';

    while(cmd != 'q') {
        scanf(" %c", &cmd);
    }

    return EXIT_SUCCESS;
}
