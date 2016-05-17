#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#define FLYING_TIME 3
#define STOP_TIME 2


typedef struct ThreadArgs {
    int airplane_id;

} thread_args_t;


static pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t landing = PTHREAD_COND_INITIALIZER;
static pthread_cond_t starting = PTHREAD_COND_INITIALIZER;
static volatile int docked_airplanes, aircraft_limit, k, waiting_to_start, waiting_to_land;
static volatile bool is_airstrip_available = true;


void mutex_lock(pthread_mutex_t *mutex)
{
    if(pthread_mutex_lock(mutex) < 0) {
        perror("Mutex could not be locked");
        exit(EXIT_FAILURE);
    }
}

void mutex_unlock(pthread_mutex_t *mutex)
{
    if(pthread_mutex_unlock(mutex) < 0) {
        perror("Mutex could not be unlocked");
        exit(EXIT_FAILURE);
    }
}

void cond_signal(pthread_cond_t *cond)
{
    if(pthread_cond_signal(cond) < 0) {
        perror("Cond signal failed");
        exit(EXIT_FAILURE);
    }
}

void cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if(pthread_cond_wait(cond, mutex) < 0) {
        perror("Cond wait failed");
        exit(EXIT_FAILURE);
    }
}


void release_airstrip()
{
    if(docked_airplanes < k) {
        if(waiting_to_land > 0) {
            cond_signal(&landing);
        } else {
            cond_signal(&starting);
        }
    } else {
        if(waiting_to_start > 0) {
            cond_signal(&starting);
        } else if(docked_airplanes < aircraft_limit) {
            cond_signal(&landing);
        }
    }
}


void request_start(int airplane_id)
{
    mutex_lock(&monitor_mutex);

    printf("Airplane %d requests start\n", airplane_id);

    while(!is_airstrip_available) {
        waiting_to_start += 1;
        cond_wait(&starting, &monitor_mutex);
        waiting_to_start -= 1;
    }
    is_airstrip_available = false;

    printf("Airplane %d is allowed to start\n", airplane_id);

    mutex_unlock(&monitor_mutex);
}


void start(int airplane_id)
{
    mutex_lock(&monitor_mutex);

    printf("Airplane %d is starting\n", airplane_id);

    docked_airplanes -= 1;
    is_airstrip_available = true;
    release_airstrip();

    printf("Airplane %d is flying\n", airplane_id);

    mutex_unlock(&monitor_mutex);
}


void request_landing(int airplane_id)
{
    mutex_lock(&monitor_mutex);

    printf("Airplane %d requests landing\n", airplane_id);

    while(!is_airstrip_available || docked_airplanes == aircraft_limit) {
        waiting_to_land += 1;
        cond_wait(&landing, &monitor_mutex);
        waiting_to_land -= 1;
    }
    is_airstrip_available = false;

    printf("Airplane %d is allowed to land\n", airplane_id);

    mutex_unlock(&monitor_mutex);
}


void land(int airplane_id)
{
    mutex_lock(&monitor_mutex);

    printf("Airplane %d is landing\n", airplane_id);

    docked_airplanes += 1;
    is_airstrip_available = true;
    release_airstrip();

    printf("Airplane %d is waiting in dock\n", airplane_id);

    mutex_unlock(&monitor_mutex);
}


void fly(int airplane_id)
{
    sleep(rand() % FLYING_TIME);
}

void stop(int airplane_id)
{
    sleep(rand() % STOP_TIME);
}


void *run_airplane(void *parameters)
{
    thread_args_t *args;
    int airplane_id;

    args = (thread_args_t *) parameters;
    airplane_id = args->airplane_id;

    while(1) {
        request_landing(airplane_id);
        land(airplane_id);
        stop(airplane_id);
        request_start(airplane_id);
        start(airplane_id);
        fly(airplane_id);
    }

    return NULL;
}


void run_aircraft(int airplanes_number)
{
    pthread_t *thread_ids;
    thread_args_t *args;
    int i;
    char cmd;

    thread_ids = malloc(sizeof(pthread_t)*airplanes_number);
    args = malloc(sizeof(thread_args_t)*airplanes_number);

    for(i=0; i<airplanes_number; i+=1) {
        args[i].airplane_id = i+1;

        pthread_create(&thread_ids[i], NULL, run_airplane, &args[i]);
    }

    cmd = '0';
    while(cmd != 'q') {
        scanf(" %c", &cmd);
    }

    for(i=0; i<airplanes_number; i+=1) {
        pthread_cancel(thread_ids[i]);
    }

    free(thread_ids);
    free(args);
}


int parse_int(char* arg)
{
    long num;
    char *error;

    num = strtol(arg, &error, 10);

    if(strlen(error)) {
        fprintf(stderr, "Argument %s is not a number.\n", arg);
        exit(EXIT_FAILURE);
    }

    if(num >= INT_MAX) {
        fprintf(stderr, "Argument %s has exceeded integer limit.\n", arg);
        exit(EXIT_FAILURE);
    }

    return (int)num;
}


int main(int argc, char* argv[])
{
    int airplanes_number;

    if(argc != 4) {
        printf("Invalid number of arguments. Usage: <N> <K> <airplanes_number>\n");
        exit(EXIT_FAILURE);
    }

    aircraft_limit = parse_int(argv[1]);
    k = parse_int(argv[2]);
    airplanes_number = parse_int(argv[3]);

    run_aircraft(airplanes_number);

    return 0;
}
