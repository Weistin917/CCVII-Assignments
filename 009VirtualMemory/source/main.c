// Read memory addresses from file and print translation
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "memio.h"
#include "random.h"

#define     HELPMSG     "\
Virtual Memory traduction simulator.\
Available arguments:\
 -V: number of virtual pages.\
 -seed: random seed for reproducibility.\
 -help: show help manual of the program (overides any other argument)\n"

int* parseArgs(int argc, char* argv[]);

/**
 * CLI arguments parsing
 * @param argc: arguments count
 * @param argv: arguments vector
 */
int* parseArgs(int argc, char* argv[]) {
    assert((argc - 1) % 2 == 0);
    int ret[2];
    unsigned int seed = 0;
    uint8_t num_vpage = 0;
    
    for (uint8_t i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-help")) {
            printf(HELPMSG);
            exit(0);
        }
        if (strcmp(argv[i], "-V")) {
            num_vpage = (uint8_t)atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-seed")) {
            seed = atoi(argv[i+1]);
        }
    }

    if (!seed) seed = (unsigned int)time(NULL);
    if (!num_vpage) num_vpage = 49;

    ret[0] = seed;
    ret[1] = num_vpage;
    
    return ret;
}

int main(int argc, char* argv[]) {
    int args[2] = parseArgs(argc, argv);

    rand_seed(args[0]);

    RAM_init((uint8_t)args[1]);
}