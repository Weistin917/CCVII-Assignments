#ifndef RAM_H_
#define RAM_H_

#include <stdint.h>

#define FRAME_EMPTY 0xFF

typedef uint8_t frame;

extern frame   *frames;
extern uint8_t  num_frames;

void RAM_init(uint8_t n);
void RAM_free(void);
void RAM_get_view(frame *buf);

#endif /* RAM_H_ */
