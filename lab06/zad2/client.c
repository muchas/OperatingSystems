#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <time.h>
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

// globals because they are used in exit handler
static int client_id;
static mqd_t server_mqd, client_mqd;
static char client_queue_name[MAX_QUEUE_NAME_LENGTH];


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


int open_server_connection(mqd_t server_mqd, mqd_t client_mqd, char* client_queue_name)
{
    queue_message_t msg;

    msg.message.type = NEW_CLIENT;
    strcpy(msg.message.queue_name, client_queue_name);

    if(mq_send(server_mqd, msg.bytes, MESSAGE_SIZE, 0) == -1) {
        fprintf(stderr, "Cannot open client connection, mq_send: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(mq_receive(client_mqd, msg.bytes, MESSAGE_SIZE+15, 0) < 0) {
        fprintf(stderr, "Cannot open client connection, msq_receive: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(msg.message.type != SERVER_ACCEPTANCE) {
        fprintf(stderr, "Server refused connection\n");
        exit(EXIT_FAILURE);
    }

    return msg.message.client_id;
}

void send_ready_status(int client_id, mqd_t server_mqd)
{
    queue_message_t msg;

    msg.message.type = CLIENT_READY;
    msg.message.client_id = client_id;

    if(mq_send(server_mqd, msg.bytes, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot send ready status, mq_send: %s\n", strerror(errno));
    }
}


void send_response(int client_id, int number, mqd_t server_mqd)
{
    queue_message_t msg;

    msg.message.type = CLIENT_RESPONSE;
    msg.message.number = number;
    msg.message.is_prime = is_prime(number);
    msg.message.client_id = client_id;

    if(mq_send(server_mqd, msg.bytes, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "Cannot send response, mq_send: %s\n", strerror(errno));
    }
}


void route_received_messages(int client_id, mqd_t client_mqd, mqd_t server_mqd)
{
    queue_message_t msg;

    send_ready_status(client_id, server_mqd);

    while(mq_receive(client_mqd, msg.bytes, MESSAGE_SIZE+15, NULL) >= 0) {
        if(msg.message.type == SERVER_RESPONSE) {
            send_response(client_id, msg.message.number, server_mqd);
            sleep(1);
            send_ready_status(client_id, server_mqd);
        } else {
            fprintf(stderr, "Received unexpected message, type: %d\n", msg.message.type);
        }
    }

    fprintf(stderr, "mq_receive: %s\n", strerror(errno));
}

void close_message_queue()
{
    queue_message_t msg;

    msg.message.type = CLOSE_QUEUE;
    msg.message.client_id = client_id;

    if(mq_send(server_mqd, msg.bytes, MESSAGE_SIZE, 0) < 0) {
        fprintf(stderr, "mq_send: %s\n", strerror(errno));
    }

    if(mq_close(client_mqd) == -1) {
        fprintf(stderr, "mq_close: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(mq_unlink(client_queue_name) == -1) {
        fprintf(stderr, "mq_unlink: %s\n", strerror(errno));
    }

    exit(EXIT_SUCCESS);
}

void generate_queue_name(char *buffer, int length)
{
    int i;

    if(length < 3) {
        fprintf(stderr, "Queue name is too short");
    }

    buffer[0] = '/';
    buffer[length-1] = '\0';

    for(i=1; i<length-1; i+=1) {
        buffer[i] = rand() % ('z' - '0') + '0';
    }
}



int main(int argc, char* argv[])
{
    char *server_queue_name;
    struct mq_attr attr;


    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <server_queue_name>\n");
        exit(EXIT_FAILURE);
    }

    server_queue_name = strdup(argv[1]);

    if(server_queue_name == NULL) {
        fprintf(stderr, "strdup: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    generate_queue_name(client_queue_name, MAX_QUEUE_NAME_LENGTH);

    // configure client queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    client_mqd = mq_open(client_queue_name, O_RDONLY | O_CREAT, 0666, &attr);
    server_mqd = mq_open(server_queue_name, O_WRONLY);

    signal(SIGINT, close_message_queue);

    if(client_mqd < 0 || server_mqd < 0) {
        fprintf(stderr, "mq_open: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    client_id = open_server_connection(server_mqd, client_mqd, client_queue_name);
    route_received_messages(client_id, client_mqd, server_mqd);

    free(server_queue_name);

    return 0;
}