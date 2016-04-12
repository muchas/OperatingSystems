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

static int client_queues[MAX_CLIENT_LIMIT];
static int new_client_id = 0;


// assign id and open queue for a new client
void open_client_connection(key_t client_key)
{
    int queue_id;
    message_t msg;

    if(new_client_id >= MAX_CLIENT_LIMIT) {
        printf("Exceeded client limit. Connection refused \n");
        return;
    }

    queue_id = msgget(client_key, 0);

    if(queue_id < 0) {
        printf("Cannot open client connection. Msgget error\n");
        return;
    }

    client_queues[new_client_id] = queue_id;

    msg.type = SERVER_ACCEPTANCE;
    msg.client_id = new_client_id;

    if(msgsnd(queue_id, &msg, MESSAGE_SIZE, 0) < 0) {
        printf("Cannot open client connection. Msgsnd error\n");
    }

    new_client_id += 1;
}


void send_random_number(int client_id)
{
    message_t msg;

    if(client_id >= new_client_id) {
        printf("Invalid client id\n");
        return;
    }

    msg.number = rand() % 1000;

    if(msgsnd(client_queues[client_id], &msg, MESSAGE_SIZE, 0) < 0) {
        printf("Cannot send random number. Msgsnd error\n");
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

        type = (message_type_t) msg.type;

        switch(type) {
            case NEW_CLIENT:
                open_client_connection((key_t) msg.client_id); // using client_id field as client_key in this type of msg
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
    int server_id, queue_id;
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

    queue_id = msgget(server_key, IPC_CREAT);

    if(queue_id < 0) {
        printf("Msgget error\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    route_received_messages(queue_id);

    return 0;
}