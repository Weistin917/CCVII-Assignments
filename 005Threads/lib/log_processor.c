#include    <fcntl.h>
#include    <semaphore.h>
#include    <stdlib.h>
#include    "log_processor.h"

sem_t *file_mutex, *ip_request_mutex, *url_visits_mutex, *errors_mutex;

/**
 * Initialize the semaphores used for synchronization.
 */
void semaphores_init() {
  file_mutex = sem_open("/file-mutex", O_CREAT, 0666, 1);
  ip_request_mutex = sem_open("/ip_mutex", O_CREAT, 0666, 1);
  url_visits_mutex = sem_open("/url_mutex", O_CREAT, 0666, 1);
  errors_mutex = sem_open("/errors_mutex", O_CREAT, 0666, 1);
}

/**
 * Cleans the semaphores.
 */
void semaphores_clean() {
  sem_unlink("/file_mutex");
  sem_unlink("/ip_mutex");
  sem_unlink("/url");
  sem_unlink("/errors_mutex");
}

/**
 * Parse a log entry to get the IP, URL and status code.
 * Needs to be thread safe, so synchronization is needed.
 * @param file: stream where the log file is opened.
 * @param ip: address to store the IP value.
 * @param url: address to store the URL value.
 * @param status: address to store the status code.
 * @return 0 if error reading the file or got end of the file, 1 otherwise.
 */
uint8_t parse_log_entry(FILE* file, char* ip, char* url, uint16_t* status) {
  char buffer[100];
  uint8_t isEnd;
  // file reading mutex
  sem_wait(file_mutex);
  isEnd = (fgets(buffer, sizeof(buffer), file) == NULL);
  sem_post(file_mutex);

  if (isEnd) return 0;
  sscanf(buffer, "%s - - %*s\"%*s %s\" %hd", ip, url, status);
  return 1;
}

/**
 * Increment the request number for the given IP.
 * Needs to be thread safe, so synchronization is needed.
 * @param ip_requests: hash table where the number of requests per IP is stored.
 * @param ip: the IP that is sendind the request.
 */
void count_ip_request(ht* ip_requests, char *ip) {
  // ip request mutex
  sem_wait(ip_request_mutex);
  void *count = ht_get(ip_requests, ip);
  sem_post(ip_request_mutex);

  if (count != NULL) {
    // Count already exists for the given IP
    uint32_t *rcount = (uint32_t *)count;
    (*rcount)++;
  } else {
    // Count doesn't exist for the given IP
    uint32_t *rcount = (uint32_t *)malloc(sizeof(uint32_t));
    *rcount = 1;
    // ip request mutex
    sem_wait(ip_request_mutex);
    ht_set(ip_requests, ip, rcount);
    sem_post(ip_request_mutex);    
  }
}

/**
 * Increment the visit number for the given URL.
 * Needs to be thread safe, so synchronization is needed.
 * @param url_visits: hash table where the number of requests per IP is stored.
 * @param url: the URL that is getting the visit.
 */
void count_url_visits(ht* url_visits, char *url) {
  // url visits mutex
  sem_wait(url_visits_mutex);
  void *count = ht_get(url_visits, url);
  sem_post(url_visits_mutex);

  if (count != NULL) {
    // Count already exists for the given URL
    uint32_t *vcount = (uint32_t *)count;
    (*vcount)++;
  } else {
    // Count doesn't exist for the given URL
    uint32_t *vcount = (uint32_t *)malloc(sizeof(uint32_t));
    *vcount = 1;
    // url visits mutex
    sem_wait(url_visits_mutex);
    ht_set(url_visits, url, vcount);
    sem_post(url_visits_mutex);
  }
}

/**
 * Increment the number of errors.
 * Needs to be thread safe, so synchronization is needed.
 * @param errors: pointer to the number of encounted errors.
 */
void count_errors(uint16_t* errors) {
  // error mutex
  sem_wait(errors_mutex);
  (*errors)++;
  sem_post(errors_mutex);
}