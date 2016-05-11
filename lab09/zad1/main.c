#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#define PHILOSOPHERS_COUNT 5
#define THINKING_TIME 5
#define EATING_TIME 5


typedef struct ThreadArgs {
    int philosopher_id;
    sem_t *butler;
    sem_t *left_fork;
    sem_t *right_fork;
} thread_args_t;


void think(int philosopher_id)
{
    printf("Philosohper no. %d is thinking\n", philosopher_id);
    sleep(rand() % THINKING_TIME);
}

void eat(int philosopher_id)
{
    printf("Philosohper no. %d is eating\n", philosopher_id);
    sleep(rand() % EATING_TIME);
    printf("Philosopher no. %d has stopped eating\n", philosopher_id);
}

void acquire_semaphore(sem_t *semaphore)
{
    if(sem_wait(semaphore) < 0) {
        perror("Could not acquire semaphore");
        exit(EXIT_FAILURE);
    }
}

void release_semaphore(sem_t *semaphore)
{
    if(sem_post(semaphore) < 0) {
        perror("Could not acquire semaphore");
        exit(EXIT_FAILURE);
    }
}


void *run_philosopher(void *parameters)
{
    thread_args_t *args;

    args = (thread_args_t *) parameters;

    printf("Start philosopher no. %d\n", args->philosopher_id);

    while(1) {
        think(args->philosopher_id);

        acquire_semaphore(args->butler);
        acquire_semaphore(args->left_fork);
        printf("Philosopher no. %d took fork no. %d\n", args->philosopher_id, args->philosopher_id);

        acquire_semaphore(args->right_fork);
        printf("Philosopher no. %d took fork no. %d\n", args->philosopher_id, (args->philosopher_id+1)%PHILOSOPHERS_COUNT);

        eat(args->philosopher_id);

        release_semaphore(args->left_fork);

        printf("Philosopher no. %d put down no. %d\n", args->philosopher_id, args->philosopher_id);
        release_semaphore(args->right_fork);

        printf("Philosopher no. %d put down no. %d\n", args->philosopher_id, (args->philosopher_id+1)%PHILOSOPHERS_COUNT);
        release_semaphore(args->butler);
    }

    return NULL;
}


int main(int argc, char* argv[])
{
    sem_t forks[PHILOSOPHERS_COUNT];
    sem_t butler;
    pthread_t threads[PHILOSOPHERS_COUNT];
    thread_args_t args[PHILOSOPHERS_COUNT];
    int i;

    srand(time(NULL));

    sem_init(&butler, 0, PHILOSOPHERS_COUNT-1);

    for(i=0; i<PHILOSOPHERS_COUNT; i+=1) {
        sem_init(&forks[i], 0, 1);
    }

    for(i=0; i<PHILOSOPHERS_COUNT; i+=1) {
        args[i].philosopher_id = i+1;
        args[i].butler = &butler;
        args[i].left_fork = &forks[i];
        args[i].right_fork = &forks[(i+1)%PHILOSOPHERS_COUNT];

        pthread_create(&threads[i], NULL, run_philosopher, &args[i]);
    }

    for(i=0; i<PHILOSOPHERS_COUNT; i+=1) {
        pthread_join(threads[i], NULL); // parameters: thread_id, returned value
    }

    return EXIT_SUCCESS;
}
