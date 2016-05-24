#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <time.h>
#define MESSAGE_MAX_LIMIT 256
#define CLIENT_MAX_NUM 100
#define CLIENT_TIMEOUT 10


typedef struct server {
    int remote_socket_fd;
    int local_socket_fd;
    int highest_fd;

    struct sockaddr_un *local_address;
    struct sockaddr_in *remote_address;

    fd_set file_descriptors;
} server_t;

typedef struct client_timeout{
    int fd;
    time_t timestamp;
    struct sockaddr* sa;
    socklen_t *sa_len;
} client_timeout_t;


int max (int a, int b) {
    return a > b ? a : b;
}


static client_timeout_t clients[CLIENT_MAX_NUM];
static int client_num=0;

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

    socket_fd = socket(socket_family, SOCK_DGRAM, 0);
    if(!socket_fd){
        fprintf(stderr, "Socket init failed\n");
        exit(1);
    }

    result = bind(socket_fd, address, size);
    if(result != 0){
        fprintf(stderr, "Socket binding failed");
        exit(1);
    }


    return socket_fd;
}


server_t *open_server(char *socket_name, int port){
    server_t *server;

    server = (server_t *) malloc(sizeof(server_t));
    server->local_address = malloc(sizeof(struct sockaddr_un));
    server->remote_address = malloc(sizeof(struct sockaddr_in));

    setup_local_address(socket_name, server->local_address);
    setup_remote_address(port, server->remote_address);

    server->local_socket_fd = initialize_socket(AF_UNIX, (struct sockaddr *) server->local_address, sizeof(*server->local_address));
    server->remote_socket_fd = initialize_socket(AF_INET, (struct sockaddr *) server->remote_address, sizeof(*server->remote_address));
    server->highest_fd = max(0, max(server->local_socket_fd, server->remote_socket_fd));

    FD_ZERO(&server->file_descriptors);
    FD_SET(server->remote_socket_fd, &server->file_descriptors);
    FD_SET(server->local_socket_fd, &server->file_descriptors);

    return server;
}


void close_server(server_t *server)
{
    free(server->local_address);
    free(server->remote_address);
    free(server);
}


int register_client(int socket_fd, fd_set *set)
{
    int client_fd;

    client_fd = accept(socket_fd, NULL, 0);
    if(client_fd < 0){
        fprintf(stderr, "Client acceptance failed\n");
        exit(1);
    }

    FD_SET(client_fd, set);

    printf("New client registered (fd: %d)\n", client_fd);


    client_timeout_t client_timeout;
    client_timeout.fd=client_fd;
    client_timeout.timestamp=time(NULL);
    clients[client_num]=client_timeout;
    client_num += 1;

    return client_fd;
}



void update_client_timeout(int client_fd){
    int i;
    for(i=0; i<client_num; i++){
        if(clients[i].fd==client_fd){
            clients[i].timestamp=time(NULL);
        }
    }
}

int read_message(int sender_fd, char *buffer, struct sockaddr* sa, socklen_t *sa_len)
{
    update_client_timeout(sender_fd);
    printf("Reading message from fd - %d\n", sender_fd);
    return recvfrom(sender_fd, buffer, MESSAGE_MAX_LIMIT, 0, sa, sa_len);
}


int send_message(int receiver_fd, char *message)
{
    printf("Sending message to - fd: %d\n", receiver_fd);
    return write(receiver_fd, message, MESSAGE_MAX_LIMIT);
}


void broadcast_message(int sender_fd, char *message, server_t *server)
{
    int fd;

    printf("Broadcasting message from fd - %d, message: %s\n", sender_fd, message);

//    for(fd=0; fd<=server->highest_fd; fd+=1) {
//        if(FD_ISSET(fd, &server->file_descriptors) && fd != sender_fd &&
//           fd != server->local_socket_fd && fd != server->remote_socket_fd) {
//            send_message(fd, message);
//        }
//    }
}


bool close_client_connection(server_t *server, int client_fd)
{
    printf("Closing %d...\n", client_fd);
    if(close(client_fd) < 0){
        perror("Client closing error\n");
        exit(1);
    }

    FD_CLR(client_fd, &server->file_descriptors);
    if(server->highest_fd == client_fd) {
        server->highest_fd -= 1;
    }

    printf("Client (fd: %d) closed.\n", client_fd);
    return true;

}

bool close_client_timeout(server_t* server, int client_fd){
    time_t timestamp = time(NULL);
    int i,j;
    bool result=false;
    for (i=0; i<client_num; i++){
        if(clients[i].timestamp+CLIENT_TIMEOUT<timestamp){
            printf("Closing fd %d, due to timeout \n", clients[i].fd);
            close_client_connection(server, clients[i].fd);
            if(client_fd==clients[i].fd) result=true;
            client_num -= 1;
            for(j=i; j<client_num; j++){
                clients[j]=clients[j+1];
            }
        }
    }
    return result;
}



void run(server_t *server)
{
    int read_bytes;
    char message_buffer[MESSAGE_MAX_LIMIT];

    int result, fd, client_fd;
    fd_set read_set;
    struct timeval tiv;
    tiv.tv_sec = 1;
    tiv.tv_usec = 0;

    printf("Server started\n");
    struct sockaddr_storage sa;
    socklen_t sa_len=sizeof(sa);

    while(true) {
        close_client_timeout(server, 0);

        read_set = server->file_descriptors;
        result = select(server->highest_fd+1, &read_set, NULL, NULL, &tiv);
        if(result < 0){
            fprintf(stderr, "Select error\n");
            exit(1);
        }

        if(FD_ISSET(server->local_socket_fd, &read_set)){
            read_bytes = read_message(server->local_socket_fd, message_buffer, (struct sockaddr*)&sa, &sa_len);
            if(sendto(server->local_socket_fd, "HI", strlen("HI"), 0, (struct sockaddr*)&sa, sa_len)<0){
                perror("send error2");
                printf("%d", sa_len);
            }
            if(read_bytes > 0) {
                broadcast_message(server->local_socket_fd, message_buffer, server);
            } else {
                fprintf(stderr, "Error reading from socket \n");
            }
        }

        if(FD_ISSET(server->remote_socket_fd, &read_set)){
            read_bytes = read_message(server->remote_socket_fd, message_buffer, (struct sockaddr*)&sa, &sa_len);
            if(sendto(server->remote_socket_fd, "HI", strlen("HI"), 0, (struct sockaddr*)&sa, sa_len)<0){
                perror("send error");
                printf("%d", sa_len);
            }

            if(read_bytes > 0) {
                broadcast_message(server->remote_socket_fd, message_buffer, server);
            } else {
                fprintf(stderr, "Error reading from socket \n");
            }
        }


    }
}


int convert_argument_to_int(char* arg){
    char* err;
    long num = strtol(arg, &err, 10);
    if(strlen(err)){
        fprintf(stderr, "Error argument %s is not number.\n", arg);
        exit(1);
    }
    if(num>=INT_MAX || num<0){
        fprintf(stderr, "Wrong number %s.\n", arg);
        exit(1);
    }
    return (int)num;
}


int main(int argc, char* argv[]){

    if(argc != 3){
        fprintf(stderr, "Usage: <port> <socket pathname>\n");
        exit(1);
    }

    int port = convert_argument_to_int(argv[1]);
    char *socket_pathname = argv[2];

    server_t *server = open_server(socket_pathname, port);
    run(server);

    return 0;
}