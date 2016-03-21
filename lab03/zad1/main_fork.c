#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/times.h>
typedef struct tms tms_t;

int counter = 0;


void print_times(clock_t real, tms_t *tms_start, tms_t *tms_end)
{
  static long clk = 0;
  if(clk == 0) {
    clk = sysconf(_SC_CLK_TCK);
  }
  printf("Real time: %7.3fs\n", real / (double) clk);
  printf("User time: %7.3fs\n", (tms_end->tms_utime - tms_start->tms_utime) / (double) clk );
  printf("System time: %7.3fs\n", (tms_end->tms_stime - tms_start->tms_stime) / (double) clk );
  printf("Child user: %7.3fs\n", (tms_end->tms_cutime - tms_start->tms_cutime) / (double) clk );
  printf("Child system: %7.3fs\n", (tms_end->tms_cstime - tms_start->tms_cstime) / (double) clk );
}


int main(int argc, char** argv)
{
    tms_t tms_start, tms_end, tms_child;
    int i, N, status;
    pid_t child_pid;
    clock_t start, end, child_start, child_end;;
    double children_real = 0;
    long clk;
    start = times(&tms_start);
    clk = sysconf(_SC_CLK_TCK);

    N = 25000;

    for(i=0; i<N; i+=1) {
        child_start = times(&tms_child);
        child_pid = fork();

        if(child_pid == 0) {
            counter += 1;
            _exit(0);
        } else if(child_pid > 0) {
            wait(NULL);
             child_end = times(&tms_child);
             children_real += (child_end - child_start) / (double) clk;
        } else {
            printf("Error\n");
        }
    }
    printf("Counter: %d\n", counter);

    end = times(&tms_end);
    print_times(end-start, &tms_start, &tms_end);
    printf("Children real time: %7.3fs\n", children_real);
    printf("---------------------------\n");

    return 0;
}