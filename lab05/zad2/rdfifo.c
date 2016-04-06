#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define BUF_SIZE 1024


void display_message(char *buffer)
{
    char read_time_buffer[15];
    time_t read_time;
    time(&read_time);

    strftime(read_time_buffer, 15, "%H:%M:%S", localtime(&read_time));

    printf("%s - %s", read_time_buffer, buffer);
}

int main(int argc, char *argv[])
{
    char *fifo_name;
    char buffer[BUF_SIZE];
    int client_handler, read_bytes;

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <FIFO name>\n");
        exit(EXIT_FAILURE);
    }

    fifo_name = strdup(argv[1]);

    if(fifo_name == NULL) {
        printf("Invalid string");
        exit(EXIT_FAILURE);
    }

    unlink(fifo_name);

    if(mkfifo(fifo_name, S_IWUSR| S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
        printf("FIFO error\n");
        exit(EXIT_FAILURE);
    }

    client_handler = open(fifo_name, O_RDONLY);

    while(1) {
        read_bytes = read(client_handler, buffer, BUF_SIZE);

        if(read_bytes > 0) {
            display_message(buffer);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(client_handler);
    unlink(fifo_name);
    free(fifo_name);

    return 0;
}