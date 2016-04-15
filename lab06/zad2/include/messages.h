#define MAX_CLIENT_LIMIT 20
#define MAX_QUEUE_NAME_LENGTH 5
#define MESSAGE_SIZE sizeof(message_t)

typedef struct Message {
    int8_t type;
    int8_t client_id;
    int16_t number;
    int8_t is_prime;
    char queue_name[MAX_QUEUE_NAME_LENGTH];
} message_t;


typedef union QueueMessage {
    char bytes[MESSAGE_SIZE+15];
    message_t message;
} queue_message_t;


typedef enum MessageType {
    NEW_CLIENT = 1,
    CLIENT_READY,
    CLIENT_RESPONSE,
    SERVER_ACCEPTANCE,
    SERVER_RESPONSE,
    CLOSE_QUEUE
} message_type_t;
