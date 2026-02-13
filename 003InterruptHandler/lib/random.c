#include "random.h"

static unsigned int seed = 12345;

/**
 * Random number generator based on the linear congruential generator
 * @return random number
 */
unsigned int rand(void) {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}