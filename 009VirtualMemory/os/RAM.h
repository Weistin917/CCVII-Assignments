#ifndef RAM_H_
#define RAM_H_

#include <stdint.h>

#define NUM_FRAMES      100
#define FREE            0
#define PREOCCUPIED     1
#define OCCUPIED        2

typedef struct frame {
    uint8_t state;
    uint8_t v_page;
} frame;

frame frames[NUM_FRAMES];

#endif  /*RAM_H_*/