#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#define BUF_SIZE 120
#define PIPE_OUT 0
#define PIPE_IN 1


void fold(unsigned N)
{
    char *buffer = malloc(sizeof(char)*N);
    int n;

    if(buffer == NULL){
        perror("Failed to allocate buffer");
        exit(1);
    }

    while(fgets(buffer, N, stdin)) {
        fputs(buffer, stdout);
        n = strlen(buffer);
        if(n == N && buffer[n-1] != '\n') {
            fputc('\n', stdout);
        }
    }

    free(buffer);
}


void convert_uppercase()
{
    char buffer[BUF_SIZE];
    int i;

    while(fgets(buffer, BUF_SIZE, stdin)) {
        for(i = 0; i < strlen(buffer); i+=1) {
            buffer[i] = toupper(buffer[i]);
        }
        fputs(buffer, stdout);
    }
}


void replace_file_descriptor(int fd, int target)
{
    if(fd == target) return;
    if(dup2(fd, target) != target) {
        printf("Dup2 error\n");
        exit(EXIT_FAILURE);
    };
    close(fd);
}


void handle_pipe(unsigned N)
{
    int fd[2];
    pid_t pid;

    if(pipe(fd) < 0) {
        printf("Pipe error\n");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) < 0) {
        printf("Fork error\n");
        exit(EXIT_FAILURE);
    } else if(pid > 0) {
        // parent process
        close(fd[PIPE_OUT]);
        replace_file_descriptor(fd[PIPE_IN], STDOUT_FILENO);
        convert_uppercase();
    } else {
        // child process
        close(fd[PIPE_IN]);
        replace_file_descriptor(fd[PIPE_OUT], STDIN_FILENO);
        fold(N);
    }
}


int main(int argc, char* argv[])
{
    int N;

    if(argc != 2) {
        printf("Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    N = atoi(argv[1]);

    handle_pipe(N);

    return 0;
}
