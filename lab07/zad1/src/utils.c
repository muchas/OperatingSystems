#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "utils.h"


static int memory_id;


int parse_int(char* arg){
    char* error;
    long num = strtol(arg, &error, 10);
    if(strlen(error)) {
        fprintf(stderr, "Argument %s is not number.\n", arg);
        exit(EXIT_FAILURE);
    }
    if(num >= INT_MAX){
        fprintf(stderr, "Argument %s has exceeded integer limit.\n", arg);
        exit(EXIT_FAILURE);
    }
    return (int)num;
}


void remove_shared_memory()
{
    if(shmctl(memory_id, IPC_RMID, 0) == -1) {
        perror("Closing error");
        exit(EXIT_FAILURE);
    }
}


key_t get_key(char *pathname, int project_id)
{
    key_t key;

    key = ftok(pathname, project_id);

    if(key < 0) {
        fprintf(stderr, "ftok: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return key;
}


int* get_shared_memory(key_t memory_key, int flags)
{
    int *memory;

    memory_id = shmget(memory_key, SHARED_MEMORY_SIZE, flags);

    if(memory_id < 0) {
        fprintf(stderr, "shmget: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memory = shmat(memory_id, NULL, 0);

    if(memory == NULL) {
        fprintf(stderr, "shmat: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // remove only in processes where memory was created
    if(flags & IPC_CREAT) {
        atexit(remove_shared_memory);
    }

    return memory;
}


void acquire_semaphore(int semaphore_id)
{
    struct sembuf sops[2];
    sops[0].sem_num = 0;        // Operate on semaphore 0
    sops[0].sem_op = 0;         // Wait for value to equal 0
    sops[0].sem_flg = 0;
    sops[1].sem_num = 0;        // Operate on semaphore 0
    sops[1].sem_op = 1;         // Increment value by one
    sops[1].sem_flg = 0;

    if (semop(semaphore_id, sops, 2) == -1) {
        perror("semoperation");
        exit(EXIT_FAILURE);
    }
}


void release_semaphore(int semaphore_id)
{
    struct sembuf sops[1];
    sops[0].sem_num = 0;        // Operate on semaphore 0
    sops[0].sem_op = -1;         // Decrement value by one
    sops[0].sem_flg = 0;
    if (semop(semaphore_id, sops, 1) == -1) {
        perror("semoperation");
        exit(EXIT_FAILURE);
    }
}
