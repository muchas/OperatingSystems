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

enum client_type_t {LOCAL, REMOTE};

typedef struct server {
    int remote_socket_fd;
    int local_socket_fd;
    int highest_fd;

    struct sockaddr_un *local_address;
    struct sockaddr_in *remote_address;

    fd_set file_descriptors;
} server_t;

typedef struct client_timeout{
    char* nickname;
    time_t timestamp;
    struct sockaddr sa;
    socklen_t sa_len;
    client_type_t type;
} client_timeout_t;


int max (int a, int b) {
    return a > b ? a : b;
}


static client_timeout_t client_timestamps[CLIENT_MAX_NUM];
static int client_num=0;

char* parse_message(char* message){
    char* end_ptr=strchr(message, '}');

    int len=end_ptr-(message+1);
    char* nickname = malloc(sizeof(char)*(len+1));
    int i;
    for(i=0; i<len; i++){
        nickname[i]=message[i+1];
    }
    nickname[i]='\0';
    return nickname;
}

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



void update_client_timeout(char* nickname){
    int i;
    for(i=0; i<client_num; i++){
        if(strcmp(client_timestamps[i].nickname, nickname)==0){
            client_timestamps[i].timestamp=time(NULL);
        }
    }
}

int read_message(int sender_fd, char *buffer, struct sockaddr* sa, socklen_t *sa_len)
{
    printf("Reading message from fd - %d\n", sender_fd);
    int len=recvfrom(sender_fd, buffer, MESSAGE_MAX_LIMIT, 0, sa, sa_len);
    char* nickname = parse_message(buffer);
    update_client_timeout(nickname);
    free(nickname);
    return len;
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

    int i;
    for (i=0; i<client_num; i++){
        if(client_timestamps[i].type == LOCAL) {
            if(sendto(server->local_socket_fd, message, MESSAGE_MAX_LIMIT, 0, &client_timestamps[i].sa, client_timestamps[i].sa_len)<0){
                perror("send error");
            }
        } else {
            if(sendto(server->remote_socket_fd, message, MESSAGE_MAX_LIMIT, 0, &client_timestamps[i].sa, client_timestamps[i].sa_len)<0){
                perror("send error");
            }
        }

    }
}




void close_client_timeout(server_t* server){
    time_t timestamp = time(NULL);
    int i,j;
    for (i=0; i<client_num; i++){
        if(client_timestamps[i].timestamp+CLIENT_TIMEOUT<timestamp){
            printf("Closing %s, due to timeout \n", client_timestamps[i].nickname);
            client_num -= 1;
            for(j=i; j<client_num; j++){
                client_timestamps[j]=client_timestamps[j+1];
            }
        }
    }
}





bool is_client_registered(char* message){
    char* nickname = parse_message(message);
    int i;
    for (i=0; i<client_num; i++){
        if (strcmp(nickname, client_timestamps[i].nickname)==0){
            return true;
        }
    }
    return false;
}

void register_client(char* message, struct sockaddr sa, socklen_t sa_len, client_type_t type){
    char* nickname = parse_message(message);
    printf("New client registered (Nickname: %s)\n", nickname);


    client_timeout_t client_timeout;
    client_timeout.nickname=nickname;
    client_timeout.timestamp=time(NULL);
    client_timeout.sa = sa;
    client_timeout.type = type;
    client_timeout.sa_len = sa_len;
    client_timestamps[client_num]=client_timeout;
    client_num += 1;

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
    struct sockaddr sa;
    socklen_t sa_len=sizeof(sa);

    while(true) {
        close_client_timeout(server);

        read_set = server->file_descriptors;
        result = select(server->highest_fd+1, &read_set, NULL, NULL, &tiv);
        if(result < 0){
            fprintf(stderr, "Select error\n");
            exit(1);
        }

        if(FD_ISSET(server->local_socket_fd, &read_set)){
            read_bytes = read_message(server->local_socket_fd, message_buffer, (struct sockaddr*)&sa, &sa_len);
            if(!is_client_registered(message_buffer)){
                register_client(message_buffer, sa, sa_len, LOCAL);
            }

            if(read_bytes > 0) {
                broadcast_message(server->local_socket_fd, message_buffer, server);
            } else {
                fprintf(stderr, "Error reading from socket \n");
            }
        }

        if(FD_ISSET(server->remote_socket_fd, &read_set)){
            read_bytes = read_message(server->remote_socket_fd, message_buffer, (struct sockaddr*)&sa, &sa_len);
            if(!is_client_registered(message_buffer)){
                register_client(message_buffer, sa, sa_len, REMOTE);
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