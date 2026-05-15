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
Virtual Memory traduction simulator.\n\
Available arguments:\n\
 -V: number of virtual pages. Default: 49 pages\n\
 -seed: random seed for reproducibility. Default: random seed based on current time\n\
 -f: file name with the batch of addresses (both hex and decimal are accepted). Default: example_addresses.txt\n\
 -help: show help manual of the program (overides any other argument)\n\n"

typedef struct {
    int seed;
    char file[50];
    uint8_t num_vpage;
} args;

args parseArgs(int argc, char* argv[]);

/**
 * CLI arguments parsing
 * @param argc: arguments count
 * @param argv: arguments vector
 */
args parseArgs(int argc, char* argv[]) {
    if (argc == 1) {
        printf(HELPMSG);
    }
    assert((argc - 1) % 2 == 0);
    args ret;
    ret.num_vpage = 49;
    ret.seed = (unsigned int)time(NULL);
    sprintf(ret.file, "example_addresses.txt");
    
    for (uint8_t i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-help")) {
            printf(HELPMSG);
            exit(0);
        }
        if (!strcmp(argv[i], "-V")) {
            ret.num_vpage = (uint8_t)atoi(argv[i+1]);
        }
        else if (!strcmp(argv[i], "-seed")) {
            ret.seed = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "-f")) {
            strcpy(ret.file, argv[i+1]);
        }
    }
    
    return ret;
}

void batch_translate(char *file) {
    FILE *f = fopen(file, "r");
    char line[20];
    uint16_t v_addr;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        v_addr = (uint16_t)strtol(line, NULL, 0);
        addr_translate(v_addr);
        printf("Press Enter to continue...");
        while (getchar() != '\n');
    }
}

int main(int argc, char* argv[]) {
    args arguments = parseArgs(argc, argv);

    printf("Working with seed = %d and %d virtual pages.\n\n", arguments.seed, arguments.num_vpage);

    rand_seed(arguments.seed);

    RAM_init(arguments.num_vpage);

    printf("\nStarting batch translate...\n");
    batch_translate(arguments.file);
    printf("Finished batch translation, exiting...\n");

    RAM_free();
}