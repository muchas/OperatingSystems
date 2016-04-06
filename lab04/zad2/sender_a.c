#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t received_signals = 0;
volatile sig_atomic_t receiving = 1;


static void handle_sigusr1(int signo)
{
    received_signals += 1;
}

static void handle_sigusr2(int signo)
{
    receiving = 0;
}

void send_signals(int pid, int number, int signal)
{
    int i;
    for(i=0; i<number; i+=1) {
        if(kill(pid, signal) < 0) {
            fputs("Kill error", stderr);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    int signals_number, catcher_pid;

    if(argc != 3) {
        fputs("Invalid number of arguments. Usage: <signals number> <catcher pid>", stderr);
        return EXIT_FAILURE;
    }

    signals_number = (int) atol(argv[1]);
    catcher_pid = (int) atol(argv[2]);


    if(signal(SIGUSR2, handle_sigusr2) == SIG_ERR) {
        fputs("An error occured while setting SIGUSR2 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    if(signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        fputs("An error occured while setting SIGUSR1 signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    send_signals(catcher_pid, signals_number, SIGUSR1);
    send_signals(catcher_pid, 1, SIGUSR2);

    while(receiving) {
        pause();
    }

    printf("Expected to receive: %d\nReceived: %d\n", signals_number, received_signals);

    return 0;
}
