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
#define USERNAME_MAX_LENGTH 15


typedef struct Connection {
    int socket_fd;
    char username[USERNAME_MAX_LENGTH];
    struct sockaddr *address;
} connection_t;


typedef struct LocalConnection {
    int socket_fd;
    char username[USERNAME_MAX_LENGTH];
    struct sockaddr_un *address;
} local_connection_t;


typedef struct RemoteConnection {
    int socket_fd;
    char username[USERNAME_MAX_LENGTH];
    struct sockaddr_in *address;
} remote_connection_t;


static connection_t *connection;


int initialize_connection(int socket_family, struct sockaddr* address, int size)
{
    int socket_fd, result;

    socket_fd = socket(socket_family, SOCK_STREAM, 0);
    check(socket_fd, "Socket init failed");

    while(connect(socket_fd, address, size) == -1) {
        if(errno == ENOENT) sleep(1);
        else {
            perror("Connection error\n");
            exit(EXIT_FAILURE);
        }
    }

    return socket_fd;

error:
    if(socket_fd) close(socket_fd);
    return -1;
}


int initialize_local_connection(struct sockaddr_un* address)
{
    return initialize_connection(AF_UNIX, (struct sockaddr *) address, sizeof(*address));
}


int initialize_remote_connection(struct sockaddr_in* address)
{
    return initialize_connection(AF_INET, (struct sockaddr *) address, sizeof(*address));
}


void setup_local_address(char *socket_name, struct sockaddr_un *address)
{
    address->sun_family = AF_UNIX;
    strcpy(address->sun_path, socket_name);
}


void setup_remote_address(char *ip, int port, struct sockaddr_in *address)
{
    int result;
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    result = inet_pton(AF_INET, ip, &address->sin_addr);

    if(result == 0) {
        perror("invalid ip address\n");
        exit(EXIT_FAILURE);
    } else if(result == -1) {
        perror("inet_pton error\n");
        exit(EXIT_FAILURE);
    }
}


local_connection_t *open_local_connection(char *username, char *socket_name)
{
    local_connection_t *connection;
    connection = malloc(sizeof(local_connection_t));
    connection->address = malloc(sizeof(struct sockaddr_un));
    strcpy(connection->username, username);

    setup_local_address(socket_name, connection->address);
    connection->socket_fd = initialize_local_connection(connection->address);
    return connection;
}


remote_connection_t *open_remote_connection(char *username, char *ip, int port)
{
    remote_connection_t *connection;
    connection = malloc(sizeof(local_connection_t));
    connection->address = malloc(sizeof(struct sockaddr_in));
    strcpy(connection->username, username);

    setup_remote_address(ip, port, connection->address);
    connection->socket_fd = initialize_remote_connection(connection->address);
    return connection;
}


int send_message(connection_t *connection, char *message)
{
    char buffer[MESSAGE_MAX_LIMIT];
    strcpy(buffer, "[");
    strcat(buffer, connection->username);
    strcat(buffer, "]: ");
    strcat(buffer, message);
    return write(connection->socket_fd, buffer, MESSAGE_MAX_LIMIT);
}


int read_message(connection_t *connection, char *buffer)
{
    return read(connection->socket_fd, buffer, MESSAGE_MAX_LIMIT);
}


void *writer(void *parameters)
{
    char buffer[MESSAGE_MAX_LIMIT];
    fd_set set;

    while(true) {
        FD_ZERO(&set);
        FD_SET(connection->socket_fd, &set);

        check( select(connection->socket_fd+1, &set, NULL, NULL, NULL) >= 0, "select failed");

        if(FD_ISSET(connection->socket_fd, &set)) {
            if(read(connection->socket_fd, &buffer, MESSAGE_MAX_LIMIT) > 0) {
                fflush(stdout);
                printf("%s", buffer);
                fflush(stdout);
            } else {
                exit(0);
            }
        }
    }

    return NULL;

error:
    return NULL;
}


void *reader(void *parameters)
{
    int current_index;
    char buffer[MESSAGE_MAX_LIMIT];

    current_index = 0;

    while(true) {
        buffer[current_index++] = getchar();

        if(buffer[current_index - 1] == '\n') {
            if(current_index != 1) {
                send_message(connection, buffer);
                printf("[%s]: %.*s\n", connection->username, current_index-1, buffer);
                memset(buffer, 0, sizeof(buffer));
            }
            current_index = 0;
        }
    }
    return NULL;
}


void exit_handler()
{
    if(connection != NULL) close(connection->socket_fd);
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[])
{
    pthread_t writer_id, reader_id;

    signal(SIGINT, exit_handler);

    if(argc == 4 && strcmp(argv[2], "local") == 0) {
        connection = (connection_t *) open_local_connection(argv[1], argv[3]);
    } else if(argc == 5 && strcmp(argv[2], "remote") == 0)
        connection = (connection_t *) open_remote_connection(argv[1], argv[3], atoi(argv[4]));
    else {
        printf("Invalid arguments: client.run <id> <local/remote> <ip port | if local>|<path | if remote>\n");
        exit(EXIT_FAILURE);
    }

    check( pthread_create(&writer_id, NULL, writer, NULL) >= 0, "thread creation failed\n");
    check( pthread_create(&reader_id, NULL, reader, NULL) >= 0, "thread creation failed\n");
    check( pthread_join(writer_id, NULL) >= 0, "thread join failed\n");
    check( pthread_join(reader_id, NULL) >= 0, "thread join failed\n");

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}
