#include    <pthread.h>
#include    <string.h>
#include    <time.h>
#include    "log_processor.h"

#define     FILE_PATH     "/home/justin/Documents/Universidad Galileo/Ciclo 7/CC7/005Threads/access_1M.log"
#define     NUM_THEADS    2
#define     MULTITHREAD   1

void* process_log(void* args);
int get_most_visited(char* most_visited);
int process_single_thread();
int process_multithread();

ht* ip_requests;
ht* url_visits;
uint16_t errors;
FILE* file;

/**
 * Function that represents the job each thread will do. 
 */
void* process_log(void* args) {
  char ip[16];
  char url[256];
  uint16_t status;
  
  while (parse_log_entry_multit(file, ip, url, &status)) {
    count_ip_request_multit(ip_requests, ip);
    count_url_visits_multit(url_visits, url);
    
    if (status >= 400 && status < 600) 
      count_errors_multit(&errors);
  }

  return NULL;
}

/**
 * Function that iterates over the url_visits hash table to get the most visited URL.
 * @param most_visited: address to store the most visited URL name
 * @return the most visited URL count
 */
int get_most_visited(char* most_visited) {
  int best_count = 0, *count;
  hti url_it = ht_iterator(url_visits);
  while (ht_next(&url_it)) {
    count = (int *)url_it.value;
    if (*(count) > best_count) {
      strcpy(most_visited, url_it.key);
      best_count = *(count);
    }
  }
  return best_count;
}

/**
 * Function to process the log file as a single thread.
 */
int process_single_thread() {
  char ip[16];
  char url[256];
  uint16_t status;

  while (parse_log_entry(file, ip, url, &status)) {
    count_ip_request(ip_requests, ip);
    count_url_visits(url_visits, url);
    
    if (status >= 400 && status < 600) 
      count_errors(&errors);
  }

  return 0;
}

/**
 * Function to process the log file with multiple threads.
 */
int process_multithread() {
  semaphores_init();

  pthread_t threads[NUM_THEADS];
  int result_code;

  printf("Creating %d threads to process log file...\n", NUM_THEADS);
  for (uint8_t i = 0; i < NUM_THEADS; i++) {
    printf("Main: Creating thread %d.\n", i);
    result_code = pthread_create(&threads[i], NULL, process_log, NULL);
    if (result_code) {
      printf("Error creating thread %d.\n", i);
      return -1;
    }
  }

  printf("Main: Waiting for threads to finish...\n");

  for (uint8_t i = 0; i < NUM_THEADS; i++) {
    result_code = pthread_join(threads[i], NULL);
    if (result_code) {
      printf("Error joining thread %d.\n", i);
      return -1;
    }
    printf("Main: Thread %d has ended.\n", i);
  }
  
  semaphores_clean();
  return 0;
}

int main(void) {
  clock_t start, end;
  double cpu_time;

  file = fopen(FILE_PATH, "r");
  ip_requests = ht_create();
  url_visits = ht_create();
  errors = 0;
  
  if (file == NULL) {
    printf("Failed to open file.\n");
    return 1;
  }
  
  char most_visited_url[256];
  int best_count = 0;

  if (MULTITHREAD) {
    printf("Main: Running in multithread.\n");
    start = clock();
    if (process_multithread()) return -1;
  } else {
    printf("Main: Running in single thread.\n");
    start = clock();
    process_single_thread();
  }
  end = clock();

  cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
  printf("CPU execution time: %f\n", cpu_time);

  printf("Total unique IPs: %d.\n", (int)ht_length(ip_requests));
  best_count = get_most_visited(most_visited_url);
  printf("Most Visited URL: %s (%d times).\n", most_visited_url, best_count);
  printf("HTTP Errors: %d.\n", errors);

  ht_destroy(ip_requests);
  ht_destroy(url_visits);
}