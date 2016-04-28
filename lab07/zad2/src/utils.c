#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "utils.h"


static int *memory;


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

void unmap_memory()
{
    munmap(memory, MEMORY_SIZE);
}


int* get_shared_memory(char *memory_name, int flags)
{
    int memory_fd;

    memory_fd = shm_open(memory_name, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if(memory_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    memory = mmap(NULL, MEMORY_SIZE, flags, MAP_SHARED, memory_fd, 0);

    if(memory == NULL) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    atexit(unmap_memory);

    return memory;
}


sem_t* get_semaphore(char* name)
{
    sem_t *semaphore;

    semaphore = sem_open(name, O_RDWR);

    if(semaphore == SEM_FAILED){
        perror("semaphore open");
        exit(EXIT_FAILURE);
    }

    return semaphore;
}

void acquire_semaphore(sem_t *semaphore)
{
    if(sem_wait(semaphore)) {
        perror("semaphore error");
        exit(EXIT_FAILURE);
    }
}


void release_semaphore(sem_t *semaphore)
{
    if(sem_post(semaphore) == -1) {
        perror("semaphore error");
        exit(EXIT_FAILURE);
    }
}
