#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>


static const int PRINT_BACKWARDS = 1 << 1;
static int counter = 1;
int flags = 0, increment_value = 1, max_num;

volatile int printing = 1;


void print(char *string, size_t length, int flags)
{
    long i;

    if(flags & PRINT_BACKWARDS) {
        for(i=length-1; i >= 0; i-=1) {
            printf("%c", string[i]);
        }
    } else {
        for(i=0; i<length; i+=1) {
            printf("%c", string[i]);
        }
    }
}


static void handle_sigtstp(int signo)
{
    counter += increment_value;

    if(counter >= max_num || counter <= 1) increment_value *= -1;

    if(counter % 2 == 0) {
        flags |= PRINT_BACKWARDS;
    } else {
        flags &= ~PRINT_BACKWARDS;
    }
}


static void handle_sigint(int signo)
{
    printf("Odebrano sygnal SIGINT\n");
    printing = 0;
}


int main(int argc, char *argv[])
{
    int i;
    size_t length;
    char *string;
    struct sigaction act;


    if(argc != 3) {
        printf("Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    string = strdup(argv[1]);
    max_num = atoi(argv[2]);

    if(string == NULL) {
        fputs("Given string could not be read.\n", stderr);
        return EXIT_FAILURE;
    }

    act.sa_handler = &handle_sigtstp;

    if(signal(SIGINT, handle_sigint) == SIG_ERR) {
        fputs("An error occurred while setting SIGINT signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    if(sigaction(SIGTSTP, &act, NULL) < 0) {
        fputs("An error occured while setting SIGTSTP signal handler.\n", stderr);
        return EXIT_FAILURE;
    }

    length = strlen(string);

    while(printing) {
        for(i=0; i<counter; i+=1) {
             print(string, length, flags);
        }
        printf("\n");
        sleep(1);
    }

    return 0;
}
