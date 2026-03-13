#ifndef   LOG_PROCESSOR_H
#define   LOG_PROCESSOR_H

#include    <stdio.h>
#include    "ht.h"

void semaphores_init();
void semaphores_clean();

uint8_t parse_log_entry(FILE* file, char* ip, char* url, uint16_t* status);

void count_ip_request(ht* ip_requests, char *ip);
void count_url_visits(ht* url_visits, char *url);
void count_errors(uint16_t* errors);

#endif  /*LOG_PROCESSOR_H*/