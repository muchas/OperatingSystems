#define MAX_CLIENT_LIMIT 20
#define MESSAGE_SIZE sizeof(message_t)-sizeof(long int)

typedef struct Message {
    long int mtype;
    int32_t client_id;
    int32_t number;
    int8_t is_prime;
} message_t;


typedef enum MessageType {
    NEW_CLIENT = 1,
    CLIENT_READY,
    CLIENT_RESPONSE,
    SERVER_ACCEPTANCE,
    SERVER_RESPONSE
} message_type_t;
