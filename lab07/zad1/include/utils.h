#include <sys/ipc.h>
#define SHARED_MEMORY_SIZE 10
#define SIZE_INDEX (SHARED_MEMORY_SIZE-1)
#define END_INDEX (SHARED_MEMORY_SIZE-2)
#define START_INDEX (SHARED_MEMORY_SIZE-3)
#define ARRAY_SIZE (SHARED_MEMORY_SIZE-3)
#define SEMAPHORE_NUM ARRAY_SIZE


int parse_int(char* arg);
key_t get_key(char* pathname, int project_id);
int* get_shared_memory(key_t memory_key, int flags);
void acquire_semaphore(int semaphore_id);
void release_semaphore(int semaphore_id);
