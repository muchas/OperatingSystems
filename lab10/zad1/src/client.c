#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h> /* superset of previous */
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
static char* username;
static int struct_size;
static int fd;



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
    if(result < 0){
        printf("invalid ip address");
        exit(1);
    }
}


connection_t *open_local_connection(char *username, char *socket_name)
{
    local_connection_t *connection;
    connection = malloc(sizeof(local_connection_t));
    connection->address = malloc(sizeof(struct sockaddr_un));
    strcpy(connection->username, username);

    setup_local_address(socket_name, connection->address);
    return (connection_t*)connection;
}


connection_t *open_remote_connection(char *username, char *ip, int port)
{
    remote_connection_t *connection;
    connection = malloc(sizeof(local_connection_t));
    connection->address = malloc(sizeof(struct sockaddr_in));
    strcpy(connection->username, username);

    setup_remote_address(ip, port, connection->address);

    return (connection_t*)connection;
}


int send_message(connection_t *connection, char *message)
{
    char buffer[MESSAGE_MAX_LIMIT];
    strcpy(buffer, "{");
    strcat(buffer, connection->username);
    strcat(buffer, "}: ");
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


        if(read(fd, &buffer, MESSAGE_MAX_LIMIT) > 0) {
            fflush(stdout);
            printf("%s", buffer);
            fflush(stdout);
        }
    }

    return NULL;

}


void *reader(void *parameters)
{
    int current_index;
    char buffer[MESSAGE_MAX_LIMIT];

    current_index = 0;
    char buffer2[MESSAGE_MAX_LIMIT];
    while(true) {
        buffer[current_index++] = getchar();

        if(buffer[current_index - 1] == '\n') {
            if(current_index != 1) {
                //printf("{%s}: %.*s\n", connection->username, current_index-1, buffer);

                strcpy(buffer2, "{");
                strcat(buffer2, connection->username);
                strcat(buffer2, "}: ");
                strcat(buffer2, buffer);
                if(sendto(fd, buffer2, MESSAGE_MAX_LIMIT, 0, connection->address, struct_size)<0){
                    perror("Sendto error");
                }
                memset(buffer, 0, sizeof(buffer));
                memset(buffer2, 0, sizeof(buffer2));
            }
            current_index = 0;
        }
    }
    return NULL;
}

int create_socket_local(char* name){
    int socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(socket_fd==-1){
        perror("Socket init failed");
        exit(1);
    }

    struct sockaddr_un address;
    unlink(name);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, name);

    if(bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("connect error");
        exit(1);
    }

    return socket_fd;
}

int create_socket_remote(int port){
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd==-1){
        perror("Socket init failed");
        exit(1);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port+11);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

//    if(bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
//        perror("connect error");
//        exit(1);
//    }

    return socket_fd;
}


int main(int argc, char *argv[])
{
    pthread_t writer_id, reader_id;
    username = argv[1];
    int socket_fd;

    if(argc == 4 && strcmp(argv[2], "local") == 0) {
        socket_fd = create_socket_local(argv[1]);
        connection = (connection_t *) open_local_connection(argv[1], argv[3]);
        struct_size = sizeof(struct sockaddr_un);
    } else if(argc == 5 && strcmp(argv[2], "remote") == 0){
        socket_fd = create_socket_remote(atoi(argv[4]));
        connection = (connection_t *) open_remote_connection(argv[1], argv[3], atoi(argv[4]));
        struct_size = sizeof(struct sockaddr_in);
    }

    else {
        printf("Usage <id> <local/remote> <ip (remote)> <port (remote)> | <path (local)>\n");
        exit(1);
    }

//    if(sendto(socket_fd, "Hello world", strlen("Hello world"), 0, connection->address, struct_size)<0){
//        perror("Sendto error");
//    }
    //char buffer[MESSAGE_MAX_LIMIT];
    //recvfrom(socket_fd, buffer, MESSAGE_MAX_LIMIT, 0, NULL, NULL);
    //printf("%s", buffer);
    fd=socket_fd;

    if( pthread_create(&writer_id, NULL, writer, NULL) < 0){
        printf("thread creation failed\n");
        exit(1);
    }
    if( pthread_create(&reader_id, NULL, reader, NULL) < 0){
        printf("thread creation failed\n");
        exit(1);
    }
    if( pthread_join(writer_id, NULL) < 0){
        printf("thread join failed\n");
        exit(1);
    }
    if( pthread_join(reader_id, NULL) < 0){
        printf("thread join failed\n");
        exit(1);
    }

    return 0;
}