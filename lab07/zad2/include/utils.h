#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#define MEMORY_SIZE 30
#define ARRAY_SIZE (MEMORY_SIZE-1)
#define READERS_COUNT_INDEX (MEMORY_SIZE-1)
#define READER_MUTEX_NAME "/reader"
#define WRITER_SEMAPHORE_NAME "/writer"


sem_t* get_semaphore(char *name);
int* get_shared_memory(char *name, int flags);
void acquire_semaphore(sem_t *semaphore);
void release_semaphore(sem_t *semaphore);