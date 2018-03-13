#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_INTERVAL 25

int main(int argc, char** argv)
{
    FILE *trouble = fopen("presfile","a+");
  if (argc < 2) {
    fprintf(trouble, "Incorrect number of arguments; usage: presblock [pid]\n");
    fclose(trouble);
    exit(EXIT_FAILURE);
  }

  // Convert text pid to int
  int proc_pid = atoi(argv[1]);
  fprintf(trouble,"Process ID: %d",proc_pid);

  // Check if pid is a sane value
  if (proc_pid > 0) {
    
    while(proc_pid == getppid()) {
      // AGENT Smith:
      // rand() is the worst PRNG ever; FFS use something else!
      int delay = rand() % MAX_INTERVAL;
      fprintf(trouble, "Delay: %d\n", delay);
      sleep(delay);
      kill(proc_pid, SIGALRM);
    }
  } else {
    fprintf(trouble, "Invalid pid! Exiting...\n");
    fclose(trouble);
    exit(EXIT_FAILURE);
  }

  // Unreachable code
  exit(EXIT_SUCCESS);
}
