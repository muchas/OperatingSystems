#define _GNU_SOURCE             /* See feature_test_macros(7) */
#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */
#include <sched.h>


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/mman.h>
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
  printf("---------------------------\n");
}


int execute_child_process(void *arg)
{
  counter += 1;
  _exit(0);
  return 0;
}


int main(int argc, char** argv)
{
    tms_t tms_start, tms_end, tms_child;
    int i, N, status;
    pid_t cpid, w;
    clock_t start, end, child_start, child_end;
    double children_real = 0;
    long clk;
    start = times(&tms_start);
    clk = sysconf(_SC_CLK_TCK);

    N = 25000;

    char *stack = mmap(NULL, STACK_SIZE, PROT_WRITE|PROT_READ,MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK,-1,0);
    char *stack_top = stack + STACK_SIZE;

    for(i=0; i<N; i+=1) {
        child_start = times(&tms_child);

        cpid = clone(execute_child_process, (void *)stack_top, CLONE_VM | CLONE_VFORK | SIGCHLD, NULL);
        // CLONE_VM - same memory space
        // CLONE_VFORK - suspend parent process until child is done
        // SIGCHLD - signal sent when child terminates

        if(cpid == -1) {
          printf("Clone error\n");
          exit(EXIT_FAILURE);
        }

        w = waitpid(cpid, &status, 0); // third param 'options': WNOHANG, WUNTRACED, WCONTINUED

        child_end = times(&tms_child);
        children_real += (child_end - child_start) / (double) clk;

        if(w == -1) {
          printf("Waitpid error\n");
          exit(EXIT_FAILURE);
        }

    }
    printf("Counter: %d\n", counter);

    munmap(stack, STACK_SIZE);

    end = times(&tms_end);  
    print_times(end-start, &tms_start, &tms_end);
    printf("Children real time: %7.3fs\n", children_real);
    printf("---------------------------\n");
    return 0;
}