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
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include "messages.h"

extern int errno;

static mqd_t client_queues[MAX_CLIENT_LIMIT], server_mqd;
static int new_client_id = 0, queue_id;
static char* queue_name;



void open_client_connection(char* queue_name)
{
    mqd_t client_mqd;
    queue_message_t msg;

    client_mqd = mq_open(queue_name, O_WRONLY);

    if(client_mqd < 0) {
        fprintf(stderr, "mq_open: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    client_queues[new_client_id] = client_mqd;

    msg.message.type = SERVER_ACCEPTANCE;
    msg.message.client_id = new_client_id;

    if(mq_send(client_mqd, msg.bytes, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot open client connection, mq_send: %s\n", strerror(errno));
    }

    new_client_id += 1;
}

void close_client_connection(int client_id)
{
    mqd_t client_mqd;

    client_mqd = client_queues[client_id];

    printf("Closing client %d queue\n", client_id);

    if(mq_close(client_mqd) == -1) {
        fprintf(stderr, "mq_close: %s\n", strerror(errno));
    }

    client_queues[client_id] = -1;
}


void close_message_queue()
{
    int i;

    for(i=0; i<new_client_id; i+=1) {
        if(client_queues[i] >= 0) {
            close_client_connection(i);
        }
    }

    if(mq_close(server_mqd) == -1) {
        fprintf(stderr, "mq_close: %s\n", strerror(errno));
    }

    if(mq_unlink(queue_name) == -1) {
        fprintf(stderr, "mq_unlink: %s\n", strerror(errno));
    }

    exit(EXIT_SUCCESS);
}


void send_random_number(int client_id)
{
    queue_message_t msg;

    if(client_id >= new_client_id) {
        printf("Invalid client id\n");
        return;
    }

    msg.message.type = SERVER_RESPONSE;
    msg.message.number = (rand() % 1000) + 1;

    if(mq_send(client_queues[client_id], msg.bytes, MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "mq_send: %s\n", strerror(errno));
    }
}


void handle_client_response(int client_id, int number, int is_prime)
{
    if(is_prime <= 0) return;

    printf("Liczba pierwsza: %d (klient: %d)\n", number, client_id);
}


void route_received_messages(mqd_t queue_mqd)
{
    queue_message_t msg;
    message_type_t type;

    while(mq_receive(queue_mqd, msg.bytes, MESSAGE_SIZE+15, NULL) >= 0) {

        type = (message_type_t) msg.message.type;

        switch(type) {
            case NEW_CLIENT:
                printf("New client, his queue name: %s\n", msg.message.queue_name);
                open_client_connection(msg.message.queue_name);
                break;
            case CLIENT_READY:
                send_random_number(msg.message.client_id);
                break;
            case CLIENT_RESPONSE:
                handle_client_response(msg.message.client_id, msg.message.number, (int)msg.message.is_prime);
                break;
            case CLOSE_QUEUE:
                close_client_connection(msg.message.client_id);
                break;
            default:
                printf("Received unknown message type %d\n", type);
        }
    }

    fprintf(stderr, "mq_receive: %s\n", strerror(errno));
}


int main(int argc, char* argv[])
{
    struct mq_attr attr;

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <queue_name>\n");
        exit(EXIT_FAILURE);
    }

    queue_name = strdup(argv[1]);

    if(queue_name == NULL) {
        printf("strdup: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // configure server queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    server_mqd = mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr);

    if(server_mqd < 0) {
        fprintf(stderr, "mq_open: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, close_message_queue);

    srand(time(NULL));

    route_received_messages(server_mqd);

    free(queue_name);

    return 0;
}