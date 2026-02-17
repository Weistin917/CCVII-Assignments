#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int child_process(int fd);

int main(void) {
  int pipe_file_descriptor[2];
  char message[20] = "I am your father.";

  if (pipe(pipe_file_descriptor)) {
    printf("Error creating the pipe. Terminating.\n");
  } else {
    printf("Pipe created successfully!\n");

    pid_t children_pid = fork();
    
    if (children_pid == -1) {
      printf("Error creating children process. No process spawned. Terminating.\n");
    } else if (children_pid) {
      // it's the parent process
      pid_t current_pid = getpid();
      printf("Parent Process: PID = %d, Child created PID = %d\n", current_pid, children_pid);

      printf("Parent Process: Writing '%s'\n", message);
      write(pipe_file_descriptor[1], message, 20);

      printf("Parent Process: Waiting for child...\n");
      waitpid(children_pid, NULL, 0);
      printf("Parent Process: Child finished execution.\n");
    } else {
      // it's the child process, calls function
      child_process(pipe_file_descriptor[0]);
    }
  }

}

/**
 * A simple child process.
 * @param fd: file descriptor for pipe unilateral communication
 * @return execution status of the child process
 */
int child_process(int fd) {
  char message[20];
  
  pid_t current_pid = getpid();
  pid_t parent_pid = getppid();
  printf("Child Process: PID = %d, my parent PID = %d\n", current_pid, parent_pid);
  
  read(fd, message, 20);
  printf("Child Process: Received '%s'\n", message);
  printf("Child Process: Nooooooooooooo\n");
}