#include "random.h"

static unsigned int _seed = 12345;

/**
 * Seeds the random number generator.
 * @param seed: initial seed value
 */
void rand_seed(unsigned int seed) {
    _seed = seed;
}

/**
 * Returns a random integer in [min, max].
 * @param min: lower bound (inclusive)
 * @param max: upper bound (inclusive)
 */
int rand_range(int min, int max) {
    _seed = _seed * 1664525u + 1013904223u;
    unsigned int r = (_seed >> 16) & 0x7FFF;
    return min + (int)(r % (unsigned int)(max - min + 1));
}
