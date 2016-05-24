#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include "dbg.h"
#define MESSAGE_MAX_LIMIT 256
#define FD_MAX_LIMIT 40
#define CLIENT_TIMEOUT 10
int max (int a, int b) { return a > b ? a : b; }


typedef struct Server {
    int remote_socket_fd;
    int local_socket_fd;
    int highest_fd;

    struct sockaddr_un *local_address;
    struct sockaddr_in *remote_address;

    fd_set file_descriptors;
} server_t;


static server_t *server; // global due to exit handler
static time_t client[FD_MAX_LIMIT];


void setup_local_address(char *socket_name, struct sockaddr_un *address)
{
    unlink(socket_name);
    address->sun_family = AF_UNIX;
    strcpy(address->sun_path, socket_name);
}


void setup_remote_address(in_port_t port, struct sockaddr_in *address)
{
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = htonl(INADDR_ANY);
}


int initialize_socket(int socket_family, struct sockaddr* address, int size)
{
    int socket_fd, result;

    socket_fd = socket(socket_family, SOCK_STREAM, 0);
    check(socket_fd, "Socket init failed");

    result = bind(socket_fd, address, size);
    check(result == 0, "Socket binding failed");

    result = listen(socket_fd, SOMAXCONN);
    check(result == 0, "Socket listening start failed");

    return socket_fd;

error:
    if(socket_fd) close(socket_fd);
    return -1;
}


int initialize_local_socket(struct sockaddr_un* address)
{
    return initialize_socket(AF_UNIX, (struct sockaddr *) address, sizeof(*address));
}


int initialize_remote_socket(struct sockaddr_in* address)
{
    return initialize_socket(AF_INET, (struct sockaddr *) address, sizeof(*address));
}


server_t *open_server(char *socket_name, int port)
{
    server_t *server;

    server = (server_t *) malloc(sizeof(server_t));
    server->local_address = malloc(sizeof(struct sockaddr_un));
    server->remote_address = malloc(sizeof(struct sockaddr_in));

    setup_local_address(socket_name, server->local_address);
    setup_remote_address(port, server->remote_address);

    server->local_socket_fd = initialize_local_socket(server->local_address);
    server->remote_socket_fd = initialize_remote_socket(server->remote_address);
    server->highest_fd = max(0, max(server->local_socket_fd, server->remote_socket_fd));

    FD_ZERO(&server->file_descriptors);
    FD_SET(server->remote_socket_fd, &server->file_descriptors);
    FD_SET(server->local_socket_fd, &server->file_descriptors);

    return server;
}


void close_server(server_t *server)
{
    int fd;

    for(fd=0; fd<=server->highest_fd; fd+=1) {
        if(FD_ISSET(fd, &server->file_descriptors)) {
            shutdown(fd, SHUT_RDWR);

            if(close(fd) < 0) {
                perror("Server closing error");
            }
        }
    }

    free(server->local_address);
    free(server->remote_address);
    free(server);
}


int register_client(int socket_fd, fd_set *set)
{
    int client_fd;

    client_fd = accept(socket_fd, NULL, 0);
    check(client_fd >= 0, "Client acceptance failed\n");

    FD_SET(client_fd, set);

    if(client_fd > FD_MAX_LIMIT) {
        printf("FD_MAX_LIMIT exceeded \n");
        return -1;
    }

    client[client_fd] = time(NULL);

    printf("New client registered (fd: %d)\n", client_fd);

    return client_fd;
error:
    return -1;
}


int read_message(int sender_fd, char *buffer)
{
    client[sender_fd] += 2000;
    printf("Reading message from client (fd: %d)\n", sender_fd);
    return read(sender_fd, buffer, MESSAGE_MAX_LIMIT);
}


int send_message(int receiver_fd, char *message)
{
    printf("Sending message to client (fd: %d)\n", receiver_fd);
    return write(receiver_fd, message, MESSAGE_MAX_LIMIT);
}


void broadcast_message(int sender_fd, char *message, server_t *server)
{
    int fd;

    printf("Broadcasting message from client (fd %d), message: %s\n", sender_fd, message);

    for(fd=0; fd<=server->highest_fd; fd+=1) {
        if(FD_ISSET(fd, &server->file_descriptors) && fd != sender_fd &&
                fd != server->local_socket_fd && fd != server->remote_socket_fd) {
            send_message(fd, message);
        }
    }
}


bool close_client_connection(server_t *server, int client_fd)
{
    printf("Closing %d...\n", client_fd);
    check(close(client_fd) >= 0, "Client closing error\n");

    FD_CLR(client_fd, &server->file_descriptors);
    if(server->highest_fd == client_fd) {
        server->highest_fd -= 1;
    }

    printf("Client (fd: %d) closed.\n", client_fd);
    return true;

error:
    return false;
}


void close_inactive_clients(server_t* server){
    int i;
    time_t timestamp = time(NULL);

    for (i=0; i<=server->highest_fd; i++){
        if(FD_ISSET(i, &server->file_descriptors) && server->local_socket_fd != i &&
                server->remote_socket_fd != i && client[i]+CLIENT_TIMEOUT < timestamp){
            printf("Closing client (fd: %d) - timeout occurred.\n", i);
            close_client_connection(server, i);
        }
    }
}


int wait_for_client(server_t *server)
{
    int result, fd, client_fd;
    fd_set read_set;
    struct timeval tiv;

    tiv.tv_sec = 1;
    tiv.tv_usec = 0;

    while(true) {
        close_inactive_clients(server);

        read_set = server->file_descriptors;

        result = select(server->highest_fd+1, &read_set, NULL, NULL, &tiv); // nfds, readfds, writefds, exceptfds, timeout
        check(result >= 0, "Select failed\n");

        for(fd=0; fd<=server->highest_fd; fd+=1) {
            if(FD_ISSET(fd, &read_set)) {
                if(fd == server->local_socket_fd) {
                    client_fd = register_client(server->local_socket_fd, &server->file_descriptors);
                } else if(fd == server->remote_socket_fd) {
                    client_fd = register_client(server->remote_socket_fd, &server->file_descriptors);
                } else {
                    return fd;
                }

                check(client_fd, "Client registration failed");

                server->highest_fd = max(server->highest_fd, client_fd);
            }
        }
    }

error:
    return -1;
}


void run_server(server_t *server)
{
    int client_fd, read_bytes;
    char message_buffer[MESSAGE_MAX_LIMIT];

    printf("Server is running!\n");

    while(true) {
        client_fd = wait_for_client(server);
        read_bytes = read_message(client_fd, message_buffer);

        if(read_bytes > 0) {
            broadcast_message(client_fd, message_buffer, server);
        } else {
            close_client_connection(server, client_fd);
        }
    }
}


int parse_int(char* arg)
{
    long number;
    char *error;
    number = strtol(arg, &error, 10);
    check(strlen(error) == 0, "Argument %s is not a number.\n", arg);
    check(number < INT_MAX, "Argument %s has exceeded integer limit.\n", arg);
    return (int)number;
error:
    exit(EXIT_FAILURE);
}


void exit_handler(int sig)
{
    if(server != NULL) close_server(server);
    exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[])
{
    int port;
    char *socket_pathname;
    bool is_valid;

    check(argc == 3, "Invalid number of arguments. Usage: <port> <socket pathname>\n");

    port = parse_int(argv[1]);
    socket_pathname = strdup(argv[2]);

    if(signal(SIGINT, exit_handler) < 0) exit(EXIT_FAILURE);

    server = open_server(socket_pathname, port);
    run_server(server);

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}
