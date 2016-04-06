#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define BUF_SIZE 990
#define PIPE_SIZE 1024


void encode_message(char *buffer, char *target)
{
    char time_buffer[15];
    time_t write_time;
    pid_t pid;

    time(&write_time);
    pid = getpid();

    strftime(time_buffer, 15, "%H:%M:%S", localtime(&write_time));
    sprintf(target, "%d - %s - %s", pid, time_buffer, buffer);
}


void send_message(int descriptor, char *buffer, char *pipe_buffer)
{
    encode_message(buffer, pipe_buffer);
    write(descriptor, pipe_buffer, PIPE_SIZE);
}


int main(int argc, char *argv[])
{
    char *fifo_name;
    char pipe_buffer[PIPE_SIZE];
    char buffer[BUF_SIZE];
    int server_handler;


    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <FIFO name>\n");
        exit(EXIT_FAILURE);
    }

    fifo_name = strdup(argv[1]);

    if(fifo_name == NULL) {
        printf("Invalid string");
        exit(EXIT_FAILURE);
    }

    server_handler = open(fifo_name, O_WRONLY);

    if(server_handler < 0) {
        printf("FIFO open error\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, BUF_SIZE, stdin)) {
        send_message(server_handler, buffer, pipe_buffer);
    }

    close(server_handler);
    free(fifo_name);
    return 0;
}