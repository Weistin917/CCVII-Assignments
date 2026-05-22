#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

/* result of one access attempt */
typedef struct {
    uint8_t hit;        /* 1 = hit, 0 = miss */
    uint8_t evicted;    /* page evicted, FRAME_EMPTY if none */
    uint8_t loaded;     /* page loaded this step */
    uint8_t frame_idx;  /* frame index that changed */
} access_result;

extern uint32_t hits;
extern uint32_t misses;

void          scheduler_init       (uint8_t *query, uint16_t len);
void          scheduler_reset      (void);
access_result scheduler_fifo       (uint8_t page, uint16_t step);
access_result scheduler_min        (uint8_t page, uint16_t step);
access_result scheduler_lru        (uint8_t page, uint16_t step);
access_result scheduler_second_chance(uint8_t page, uint16_t step);

#endif /* SCHEDULER_H_ */
