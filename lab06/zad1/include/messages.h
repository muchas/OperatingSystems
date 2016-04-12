#define MAX_CLIENT_LIMIT 20
#define MESSAGE_SIZE 72


typedef struct Message {
    long type;
    int32_t client_id;
    int32_t number;
    int8_t is_prime;
} message_t;


typedef enum MessageType {
    NEW_CLIENT, CLIENT_READY, CLIENT_RESPONSE,
    SERVER_ACCEPTANCE, SERVER_RESPONSE
} message_type_t;
