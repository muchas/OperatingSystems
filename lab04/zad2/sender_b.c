#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile int signals_number;
volatile pid_t catcher_pid;
volatile sig_atomic_t received_signals = 0;
volatile sig_atomic_t sent_signals = 0;
volatile sig_atomic_t receiving = 1;


void send_signal(int pid, int signal)
{
    if(kill(pid, signal) < 0) {
        fputs("Kill function error", stderr);
        exit(EXIT_FAILURE);
    }
}

static void handle_sigusr1(int signo)
{
    received_signals += 1;

    if(sent_signals < signals_number) {
        send_signal(catcher_pid, SIGUSR1);
        sent_signals += 1;
    } else if(sent_signals == signals_number) {
        send_signal(catcher_pid, SIGUSR2);
    }
}

static void handle_sigusr2(int signo)
{
    receiving = 0;
}


int main(int argc, char *argv[])
{
    if(argc != 3) {
        fputs("Invalid number of arguments. Usage: <signals number> <catcher pid>", stderr);
        return EXIT_FAILURE;
    }

    signals_number = (int) atol(argv[1]);
    catcher_pid = (pid_t) atol(argv[2]);

    if(signal(SIGUSR2, handle_sigusr2) == SIG_ERR) {
        fputs("An error occured while setting SIGUSR2 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    if(signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        fputs("An error occured while setting SIGUSR1 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    send_signal(catcher_pid, SIGUSR1);
    sent_signals += 1;

    while(receiving) {
        pause();
    }

    printf("Expected to receive: %d\nReceived: %d\n", signals_number, received_signals);

    return 0;
}
