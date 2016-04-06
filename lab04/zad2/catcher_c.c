#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t received_signals = 0;
volatile sig_atomic_t receiving = 1;
volatile sig_atomic_t sender_pid = -1;


void send_signals(int pid, int number, int signal)
{
    union sigval value;
    int i;
    for(i=0; i<number; i+=1) {
        if(sigqueue(pid, signal, value) < 0) {
            fputs("Kill error", stderr);
            exit(EXIT_FAILURE);
        }
    }
}


static void handle_sigusr1(int sig, siginfo_t* info, void* context)
{
    if(sender_pid < 0) {
        sender_pid = info->si_pid;
    }

//    printf("Odebralem SIGUSR1\n");

    received_signals += 1;
}


static void handle_sigusr2(int sig, siginfo_t* info, void* context)
{
    if(sender_pid < 0) {
        sender_pid = info->si_pid;
    }

//    printf("Odebralem SIGUSR2\n");

    receiving = 0;
}


int main(int argc, char *argv[])
{
    struct sigaction first_action, second_action;

    second_action.sa_sigaction = &handle_sigusr2;
    second_action.sa_flags = SA_SIGINFO;
    sigemptyset(&second_action.sa_mask);


    if(sigaction(SIGRTMIN+3, &second_action, NULL) < 0) {
        fputs("An error occured while setting SIGUSR2 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    first_action.sa_sigaction = &handle_sigusr1;
    first_action.sa_flags = SA_SIGINFO;
    sigemptyset(&first_action.sa_mask);

    if(sigaction(SIGRTMIN+1, &first_action, NULL) < 0) {
        fputs("An error occured while setting SIGUSR1 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    while(receiving) {
        pause();
    }

 //   printf("Koncze transmisje, wysylam sygnaly do sendera\n");

    send_signals(sender_pid, received_signals, SIGRTMIN+1);
    send_signals(sender_pid, 1, SIGRTMIN+3);

    return 0;
}
