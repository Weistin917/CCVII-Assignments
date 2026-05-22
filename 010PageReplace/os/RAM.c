#include "RAM.h"
#include <stdlib.h>
#include <string.h>

frame   *frames     = NULL;
uint8_t  num_frames = 0;

/**
 * Allocates and initializes the physical frame array.
 * @param n: number of physical frames
 */
void RAM_init(uint8_t n) {
    num_frames = n;
    frames     = malloc(n * sizeof(frame));
    memset(frames, FRAME_EMPTY, n * sizeof(frame));
}

/**
 * Frees the physical frame array.
 */
void RAM_free(void) {
    free(frames);
    frames = NULL;
}

/**
 * Copies current frame state into caller-supplied buffer.
 * @param buf: output buffer of size num_frames
 */
void RAM_get_view(frame *buf) {
    memcpy(buf, frames, num_frames * sizeof(frame));
}