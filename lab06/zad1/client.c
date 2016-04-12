#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "messages.h"


int is_prime(int number)
{
    return 1;
}


void open_server_connection(int server_queue_id, key_t client_key)
{
    message_t msg;

    msg.type = NEW_CLIENT;
    msg.client_id = (int32_t) client_key;

    if(msgsnd(server_queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        printf("Cannot open client connection. Msgsnd error\n");
    }
}


void route_received_messages(int queue_id)
{
    int client_id;
    message_t msg;
    message_type_t type;
    key_t client_key;

    while(msgrcv(queue_id, &msg, MESSAGE_SIZE, 0, 0) >= 0) {

        type = (message_type_t) msg.type;

        switch(type) {
            case SERVER_ACCEPTANCE:
                client_key = (key_t) msg.client_id; // using client_id field as client_key in this type of msg
                open_client_connection(client_key);
                break;
            case SERVER_RESPONSE:
                client_id = msg.client_id;
                send_random_number(client_id);
                break;
            default:
                printf("Received unknown message type %d\n", type);
        }
    }
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
    int server_id, server_queue_id;
    key_t server_key;

    if(argc != 3) {
        printf("Invalid number of arguments. Usage: <pathname> <server_id>\n");
        exit(EXIT_FAILURE);
    }

    pathname = strdup(argv[1]);

    if(pathname == NULL) {
        printf("Invalid string");
        exit(EXIT_FAILURE);
    }

    server_id = parse_int(argv[2]);
    server_key = ftok(pathname, server_id);

    if(server_key < 0) {
        printf("Ftok error\n");
        exit(EXIT_FAILURE);
    }

    server_queue_id = msgget(server_key, 0);

    if(server_queue_id < 0) {
        printf("Msgget error\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    route_received_messages(queue_id);

    return 0;
}