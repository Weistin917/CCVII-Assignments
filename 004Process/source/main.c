#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main(void) {
  pid_t children_pid = fork();

  if (children_pid == -1) {
    
  } else if (children_pid) {
    pid_t current_pid = getpid();
    printf("Parent Process: PID = %d\n", current_pid);
    printf("Child created: PID = %d\n", children_pid);
    sleep(1);
  } else {
    pid_t current_pid = getpid();
    pid_t parent_pid = getppid();
    printf("Child Process: PID = %d, My Parent PID = %d\n", current_pid, parent_pid);
    sleep(1);
  }
}