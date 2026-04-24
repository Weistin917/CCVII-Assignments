#include    <fcntl.h>
#include    <semaphore.h>
#include    <stdlib.h>
#include    "log_processor.h"

#define     FILE_SEM_NAME   "/file-mutex"
#define     IP_SEM_NAME     "/ip_mutex"
#define     URL_SEM_NAME    "/url_mutex"
#define     ERROR_SEM_NAME      "/errors_mutex"

sem_t *file_mutex, *ip_request_mutex, *url_visits_mutex, *errors_mutex;

/**
 * Initialize the semaphores used for synchronization.
 */
void semaphores_init() {
  file_mutex = sem_open(FILE_SEM_NAME, O_CREAT, 0666, 1);
  ip_request_mutex = sem_open(IP_SEM_NAME, O_CREAT, 0666, 1);
  url_visits_mutex = sem_open(URL_SEM_NAME, O_CREAT, 0666, 1);
  errors_mutex = sem_open(ERROR_SEM_NAME, O_CREAT, 0666, 1);
}

/**
 * Cleans the semaphores.
 */
void semaphores_clean() {
  sem_unlink(FILE_SEM_NAME);
  sem_unlink(IP_SEM_NAME);
  sem_unlink(URL_SEM_NAME);
  sem_unlink(ERROR_SEM_NAME);
}

/**
 * Parse a log entry to get the IP, URL and status code.
 * @param file: stream where the log file is opened.
 * @param ip: address to store the IP value.
 * @param url: address to store the URL value.
 * @param status: address to store the status code.
 * @return 0 if error reading the file or got end of the file, 1 otherwise.
 */
uint8_t parse_log_entry(FILE* file, char* ip, char* url, uint16_t* status) {
  char buffer[100];
  uint8_t isEnd;

  isEnd = (fgets(buffer, sizeof(buffer), file) == NULL);

  if (isEnd) return 0;
  sscanf(buffer, "%16s - - %*s \"%*s %256[^\"]\" %hd", ip, url, status);
  return 1;
}

/**
 * Parse log entry function for multithreading.
 * Needs to be thread safe, so synchronization is needed.
 */
uint8_t parse_log_entry_multit(FILE* file, char* ip, char* url, uint16_t* status) {
  char buffer[100];
  uint8_t isEnd;
  // file reading mutex
  sem_wait(file_mutex);
  isEnd = (fgets(buffer, sizeof(buffer), file) == NULL);
  sem_post(file_mutex);

  if (isEnd) return 0;
  sscanf(buffer, "%16s - - %*s \"%*s %256[^\"]\" %hd", ip, url, status);
  return 1;
}

/**
 * Increment the request number for the given IP.
 * @param ip_requests: hash table where the number of requests per IP is stored.
 * @param ip: the IP that is sendind the request.
 */
void count_ip_request(ht* ip_requests, char *ip) {
  void *count = ht_get(ip_requests, ip);

  if (count != NULL) {
    // Count already exists for the given IP
    uint32_t *rcount = (uint32_t *)count;
    (*rcount)++;
  } else {
    // Count doesn't exist for the given IP
    uint32_t *rcount = (uint32_t *)malloc(sizeof(uint32_t));
    *rcount = 1;
    ht_set(ip_requests, ip, rcount);
  }
}

/**
 * Increment request number function for multithreading
 * Needs to be thread safe, so synchronization is needed.
 */
void count_ip_request_multit(ht* ip_requests, char *ip) {
  // ip request mutex
  sem_wait(ip_request_mutex);
  void *count = ht_get(ip_requests, ip);
  
  if (count != NULL) {
    // Count already exists for the given IP
    uint32_t *rcount = (uint32_t *)count;
    (*rcount)++;
    sem_post(ip_request_mutex);

  } else {
    // Count doesn't exist for the given IP
    uint32_t *rcount = (uint32_t *)malloc(sizeof(uint32_t));
    *rcount = 1;
    
    ht_set(ip_requests, ip, rcount);
    sem_post(ip_request_mutex);    
  }
}

/**
 * Increment the visit number for the given URL.
 * @param url_visits: hash table where the number of requests per IP is stored.
 * @param url: the URL that is getting the visit.
 */
void count_url_visits(ht* url_visits, char *url) {
  void *count = ht_get(url_visits, url);

  if (count != NULL) {
    // Count already exists for the given URL
    uint32_t *vcount = (uint32_t *)count;
    (*vcount)++;
  } else {
    // Count doesn't exist for the given URL
    uint32_t *vcount = (uint32_t *)malloc(sizeof(uint32_t));
    *vcount = 1;
    ht_set(url_visits, url, vcount);
  }
}

/**
 * Increment visit number function for multithreading.
 * Needs to be thread safe, so synchronization is needed.
 */
void count_url_visits_multit(ht* url_visits, char *url) {
  // url visits mutex
  sem_wait(url_visits_mutex);
  void *count = ht_get(url_visits, url);

  if (count != NULL) {
    // Count already exists for the given URL
    uint32_t *vcount = (uint32_t *)count;
    (*vcount)++;
    sem_post(url_visits_mutex);
  } else {
    // Count doesn't exist for the given URL
    uint32_t *vcount = (uint32_t *)malloc(sizeof(uint32_t));
    *vcount = 1;
    
    ht_set(url_visits, url, vcount);
    sem_post(url_visits_mutex);
  }
}

/**
 * Increment the number of errors.
 * @param errors: pointer to the number of encounted errors.
 */
void count_errors(uint16_t* errors) {
  (*errors)++;
}

/**
 * Increment error number for multithreading.
 * Needs to be thread safe, so synchronization is needed.
 */
void count_errors_multit(uint16_t* errors) {
  // error mutex
  sem_wait(errors_mutex);
  (*errors)++;
  sem_post(errors_mutex);
}