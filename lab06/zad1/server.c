#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "messages.h"

extern int errno;

static int client_queues[MAX_CLIENT_LIMIT];
static int new_client_id = 0;
static int queue_id;



void open_client_connection(int32_t queue_id)
{
    message_t msg;

    if(new_client_id >= MAX_CLIENT_LIMIT) {
        printf("Exceeded client limit. Connection refused \n");
        return;
    }

    client_queues[new_client_id] = queue_id;

    msg.mtype = SERVER_ACCEPTANCE;
    msg.client_id = new_client_id;

    if(msgsnd(queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        printf("Cannot open client connection. Msgsnd error\n");
    }

    new_client_id += 1;
}


void close_message_queue()
{
    if(msgctl(queue_id, IPC_RMID, 0) == -1){
        fprintf(stderr, "msgctl: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


void send_random_number(int client_id)
{
    message_t msg;

    if(client_id >= new_client_id) {
        printf("Invalid client id\n");
        return;
    }

    msg.mtype = SERVER_RESPONSE;
    msg.number = (rand() % 1000) + 1;

    if(msgsnd(client_queues[client_id], &msg, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot send random number, msgsnd: %s\n", strerror(errno));
    }
}


void handle_client_response(int client_id, int number, int is_prime)
{
    if(is_prime <= 0) return;

    printf("Liczba pierwsza: %d (klient: %d)\n", number, client_id);
}


void route_received_messages(int queue_id)
{
    message_t msg;
    message_type_t type;

    while(msgrcv(queue_id, &msg, MESSAGE_SIZE, 0, 0) >= 0) {

        type = (message_type_t) msg.mtype;

        switch(type) {
            case NEW_CLIENT:
                printf("New client, his queue id: %d\n", msg.client_id);
                open_client_connection(msg.client_id); // using client_id as client_queue_id
                break;
            case CLIENT_READY:
                send_random_number(msg.client_id);
                break;
            case CLIENT_RESPONSE:
                handle_client_response(msg.client_id, msg.number, (int)msg.is_prime);
                break;
            default:
                printf("Received unknown message type %d\n", type);
        }
    }

    fprintf(stderr, "msgrcv: %s\n", strerror(errno));
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
    int server_id;
    key_t server_key;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <pathname> <server_id>\n");
        exit(EXIT_FAILURE);
    }

    pathname = strdup(argv[1]);

    if(pathname == NULL) {
        printf("strdup: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    server_id = parse_int(argv[2]);
    server_key = ftok(pathname, server_id);

    if(server_key < 0) {
        fprintf(stderr, "ftok: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server key: %d\n", (int) server_key);

    queue_id = msgget(server_key, IPC_CREAT | S_IWUSR| S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    signal(SIGINT, close_message_queue);

    if(queue_id < 0) {
        fprintf(stderr, "msgget: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    route_received_messages(queue_id);

    return 0;
}