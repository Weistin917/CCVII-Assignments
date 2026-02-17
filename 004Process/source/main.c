#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void) {
  pid_t children_pid = fork();

  if (children_pid == -1) {
    printf("Error creating children process. No process spawned. Terminating.\n");
  } else if (children_pid) {
    pid_t current_pid = getpid();
    printf("Parent Process: PID = %d, Child created PID = %d\n", current_pid, children_pid);
    printf("Parent Process: Waiting for child...\n");
    waitpid(children_pid, NULL, 0);
    printf("Parent Process: Child finished execution.\n");
  } else {
    pid_t current_pid = getpid();
    pid_t parent_pid = getppid();
    printf("Child Process: PID = %d, My Parent PID = %d\n", current_pid, parent_pid);
    sleep(1);
  }
}