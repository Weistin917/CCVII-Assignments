#ifndef RAM_H_
#define RAM_H_

#include "mmu.h"

#define NUM_FRAMES      100
#define FREE            0
#define PREOCCUPIED     1
#define OCCUPIED        2

frame frames[NUM_FRAMES];

#endif  /*RAM_H_*/