#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "utils.h"


int main(int argc, char* argv[])
{
    int memory_fd;
    sem_t *writer_semaphore, *reader_semaphore;
    char cmd;

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <memory_name>\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", argv[1]);

    memory_fd = shm_open(argv[1], O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if(memory_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(memory_fd, MEMORY_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    reader_semaphore = sem_open(READER_MUTEX_NAME, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);

    if(reader_semaphore == SEM_FAILED){
        perror("reader semaphore open");
        exit(EXIT_FAILURE);
    }

    writer_semaphore = sem_open(WRITER_SEMAPHORE_NAME, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);

    if(writer_semaphore == SEM_FAILED){
        perror("writer semaphore open");
        exit(EXIT_FAILURE);
    }

    do {
        scanf(" %c", &cmd);
    } while(cmd != 'q');

    close(memory_fd);

    sem_close(writer_semaphore);
    sem_close(reader_semaphore);

    sem_unlink(READER_MUTEX_NAME);
    sem_unlink(WRITER_SEMAPHORE_NAME);

    return EXIT_SUCCESS;
}
