#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#define BUF_SIZE 120
#define PIPE_OUT 0
#define PIPE_IN 1


void fold(unsigned N)
{
    char buffer[BUF_SIZE];
    int i, word_length;

    word_length = 0;

    while(fgets(buffer, BUF_SIZE, stdin)) {
        for(i=0; i<strlen(buffer); i+=1) {
            if(buffer[i] == '\n') {
                word_length = 0;
            }

            if(word_length >= N) {
                fputc('\n', stdout);
                word_length = 0;
            }

            fputc(buffer[i], stdout);
            word_length += 1;
        }
    }
}


void convert_uppercase()
{
    char buffer[BUF_SIZE];
    int i;

    while(fgets(buffer, BUF_SIZE, stdin)) {
        for(i=0; i<strlen(buffer); i+=1) {
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
    }
    close(fd);
}


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


void handle_pipe(unsigned N)
{
    int fd[2];
    pid_t pid, result;

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
        printf("Invalid number of arguments. Usage: <N>\n");
        return EXIT_FAILURE;
    }

    N = parse_int(argv[1]);

    handle_pipe(N);

    return 0;
}
