# Lab04 - Process Creation and Handling
Assignment focused on the creation of processes, handling and communication between processes on the Linux OS.  
The different versions of the program are recorded in the following document.

---
## Part 1. Creating a new process.
A program that spawns a child process, then both parent and child displays their PID.
```
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
```
Output:  
![Output of Part 1](./assets/output1.png)
### Process Creation Description
Using the `fork()` function to create a new process, the calling process is duplicated, running at a separate memory space but with the same content. For the parent process (the calling process), `fork()` will return the children process' ID if successful, otherwise -1. For the created children process, `fork()` will return 0. This is why in the main program, what each process should do is decided by the conditional checking the value of `childre_pid`, where the return value of `fork()` is stored.

---
## Part 2. Synchronizing parent and children process.
To the previous program, added synchronizing of processes. Parent process waits for the child process to terminate.
```
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
```
Output:  
![Output of Part 2](./assets/output2.png)
### Process Synchronization Description
After the creation of the children process, the parent process calls the syscall `waitpid()` to suspend execution until any change of state happens on the children process. The default state change it will wait for is to be terminated. If the parent process doesn't wait, it will continue it's execution and terminate. In this case, the child process will be adopted by the init system of Linux to automatically perform a wait on the child and remove it.

---
## Part 3. Inter-process communication.
The parent process is able to maintain a one way communication with the children processes through a pipe.
```
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
```
Output:  
![Output of Part 3](./assets/output3.png)
### Inter-process Communication Description
The communication between processes can be achieved by using a pipe. The pipe is created using the syscall `pipe(int *fd)`, which puts the read and write file descriptors to the given array `fd`. The file descriptors acts as unique identifiers to the I/O resources, in this case the pipe. Afterwards, the parent process can use the write end of the pipe `fd[1]` to send the message, using the syscall `write()`. The children can receive the message using the syscall `read()` and the read end of the pipe `fd[0]`.