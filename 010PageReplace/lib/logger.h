#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>

void logger_init(uint8_t num_frames, uint8_t *query, uint16_t len);
void logger_run (const char *algo);   /* "fifo" | "min" | "lru" | "second_chance" | "all" */
void logger_free(void);

#endif /* LOGGER_H_ */
