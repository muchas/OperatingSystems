#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <signal.h>
#include "messages.h"


extern int errno;
static int client_queue_id;


int is_prime(int number)
{
    int limit, i;

    if(number <= 1 || number % 2 == 0) return 0;

    limit = floor(sqrt(number));

    for(i = 3; i < limit; i += 2) {
        if(number % i == 0) return 0;
    }
    return 1;
}


int open_server_connection(int server_queue_id, int client_queue_id)
{
    message_t msg;

    msg.mtype = NEW_CLIENT;
    msg.client_id = (int32_t) client_queue_id;

    printf("Server queue id: %d\nClient queue id: %d\nMsg size: %d\n", server_queue_id, client_queue_id, MESSAGE_SIZE);

    if(msgsnd(server_queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot open client connection, msgsnd: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(msgrcv(client_queue_id, &msg, MESSAGE_SIZE, SERVER_ACCEPTANCE, 0) < 0) {
        fprintf(stderr, "Cannot open client connection, msgrcv: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return msg.client_id;
}

void send_ready_status(int client_id, int server_queue_id)
{
    message_t msg;

    msg.mtype = CLIENT_READY;
    msg.client_id = client_id;

    if(msgsnd(server_queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot send ready status, msgsnd: %s\n", strerror(errno));
    }
}


void send_response(int client_id, int number, int server_queue_id)
{
    message_t msg;

    msg.mtype = CLIENT_RESPONSE;
    msg.number = number;
    msg.is_prime = is_prime(number);
    msg.client_id = client_id;

    if(msgsnd(server_queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot send response, msgsnd: %s\n", strerror(errno));
    }
}


void route_received_messages(int client_id, int queue_id, int server_queue_id)
{
    message_t msg;

    send_ready_status(client_id, server_queue_id);

    while(msgrcv(queue_id, &msg, MESSAGE_SIZE, SERVER_RESPONSE, 0) >= 0) {
        send_response(client_id, msg.number, server_queue_id);
        sleep(1);
        send_ready_status(client_id, server_queue_id);
    }
}

void close_message_queue(int signal)
{
    if(msgctl(client_queue_id, IPC_RMID, 0) == -1){
        fprintf(stderr, "msgctl: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
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


int main(int argc, char* argv[])
{
    char* pathname;
    int server_id, server_queue_id, client_id;
    key_t server_key;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <pathname> <server_id>\n");
        exit(EXIT_FAILURE);
    }

    pathname = strdup(argv[1]);

    if(pathname == NULL) {
        fprintf(stderr, "strdup: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    server_id = parse_int(argv[2]);
    server_key = ftok(pathname, server_id);

    if(server_key < 0) {
        fprintf(stderr, "ftok: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    server_queue_id = msgget(server_key, S_IWUSR| S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    client_queue_id = msgget(IPC_PRIVATE, S_IWUSR| S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    signal(SIGINT, close_message_queue);

    if(server_queue_id < 0 || client_queue_id < 0) {
        fprintf(stderr, "msgget: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    client_id = open_server_connection(server_queue_id, client_queue_id);
    route_received_messages(client_id, client_queue_id, server_queue_id);

    return 0;
}