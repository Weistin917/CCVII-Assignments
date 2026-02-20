#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>

#define   CHILD_PROCESSES   3

char *messages[3] = {"Bulbasaur", "Charmander", "Squirtle"};

int child_process(int child_id, int mem_id);

int main(void) {
  int i;
  pid_t child_ids[CHILD_PROCESSES];
  pid_t child_pid, current_pid = getpid();
  key_t key = 4951;
  char *shmem_pointer;

  printf("> Parent Process: PID = %d\n", current_pid);
  printf("> Parent Process: Allocating shared memory...\n");
  int mem_id = shmget(key, CHILD_PROCESSES * sizeof(char[12]), IPC_CREAT | 0600);
  if (mem_id == -1) {
    printf("Error allocating shared memory: %s\n", strerror(errno));
    return -1;
  }
  shmem_pointer = shmat(mem_id, NULL, 0);
  if (shmem_pointer == (void *) -1) {
    printf("Error attaching memory: %s\n", strerror(errno));
    return -1;
    
  }
  printf("> Parent Process: Shared memory allocated MEMID = %d\n", mem_id);
  // copy messages to memory space
  memcpy(shmem_pointer, messages[0], strlen(messages[0]) + 1);
  printf("> Parent Process: Writing '%s'\n", shmem_pointer);
  memcpy(shmem_pointer + 12, messages[1], strlen(messages[1]) + 1);
  printf("> Parent Process: Writing '%s'\n", shmem_pointer + 12);
  memcpy(shmem_pointer + 24, messages[2], strlen(messages[2]) + 1);
  printf("> Parent Process: Writing '%s'\n", shmem_pointer + 24);
  printf("> Parent Process: Spawning children...\n");

  for (i = 0; i < CHILD_PROCESSES; i++) {
    child_pid = fork();

    if (child_pid == -1) {
      printf("Error creating children process. No process spawned. Terminating.\n");
    } else if (!child_pid) {
      // child_pid == 0, child process
      child_process(i, mem_id);
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
    printf("> Parent Process: Children finished execution.\n");

    if (!shmdt(shmem_pointer)) {
      printf("> Parent Process: Share memory detached.\n");
    } else {
      printf("> Parent Process: Some error happened in memory detach.\n");
    }
  }
}

/**
 * A simple child process.
 * @param child_id: child id number relative to the parent
 * @param mem_id: shared memory id
 * @return execution status of the child process
 */
int child_process(int child_id, int mem_id) {
  pid_t current_pid = getpid();
  pid_t parent_pid = getppid();
  printf("- Child %d: PID = %d, my parent PID = %d\n", child_id, current_pid, parent_pid);
  printf("- Child %d: Allocating shared memory...\n", child_id);
  char *shared_mem = shmat(mem_id, NULL, SHM_RDONLY);
  printf("- Child %d: Reading '%s'\n", child_id, shared_mem + child_id * 12);
  if (!shmdt(shared_mem)) {
    printf("- Child %d: Share memory detached.\n", child_id);
  } else {
    printf("- Child %d: Some error happened in memory detach.\n", child_id);
  }
}