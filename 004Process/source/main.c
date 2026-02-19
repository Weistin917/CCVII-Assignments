#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#define   CHILD_PROCESSES   3

int child_process(int child_id);

int main(void) {
  int i;
  pid_t child_ids[CHILD_PROCESSES];
  pid_t child_pid, current_pid = getpid();

  printf("Parent Process: PID = %d\n", current_pid);
  printf("Parent Process: Spawning children...\n");

  for (i = 0; i < CHILD_PROCESSES; i++) {
    child_pid = fork();

    if (child_pid == -1) {
      printf("Error creating children process. No process spawned. Terminating.\n");
    } else if (!child_pid) {
      // child_pid == 0, child process
      child_process(i);
      break;
    } else {
      // parent process, store the child pid
      child_ids[i] = child_pid;
    }
  }

  if (child_pid) {
    for (i = 0; i < CHILD_PROCESSES; i++) {
      // parent process wait for the children
      waitpid(child_ids[i], NULL, 0);
    }
    printf("Parent Process: Children finished execution.\n");
  }
}

/**
 * A simple child process.
 * @param child_id: child id number relative to the parent
 * @return execution status of the child process
 */
int child_process(int child_id) {
  pid_t current_pid = getpid();
  pid_t parent_pid = getppid();
  printf("Child %d: PID = %d, my parent PID = %d\n", child_id, current_pid, parent_pid);
}