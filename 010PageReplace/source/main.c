#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "../lib/logger.h"

#define HELPMSG \
"Memory page replacement simulator.\n"\
"Available arguments:\n"\
" -N      : number of physical frames. Default: 3\n"\
" -query  : page reference sequence, space-separated (e.g. -query \"7 0 1 2\")\n"\
" -f      : path to file with one page number per line\n"\
" -a      : algorithm: fifo | min | lru | second_chance | all. Default: all\n"\
" -help   : show this help message\n\n"

#define MAX_QUERY 1024

typedef struct {
    uint8_t  query[MAX_QUERY];
    uint16_t query_len;
    uint8_t  num_frames;
    char     algo[32];
} args;

static uint16_t read_file(uint8_t *buf, const char *file) {
    FILE   *f = fopen(file, "r");
    char    line[20];
    uint16_t i = 0;
    if (!f) { fprintf(stderr, "Cannot open file: %s\n", file); exit(-1); }
    while (fgets(line, sizeof(line), f) && i < MAX_QUERY) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;
        long v = strtol(line, NULL, 0);
        if (v < 0) { fprintf(stderr, "Invalid page number: %s\n", line); exit(-1); }
        buf[i++] = (uint8_t)v;
    }
    fclose(f);
    return i;
}

static uint16_t parse_query_str(uint8_t *buf, const char *s) {
    uint16_t i = 0;
    char     tmp[MAX_QUERY * 4];
    strncpy(tmp, s, sizeof(tmp) - 1);
    char *tok = strtok(tmp, " ");
    while (tok && i < MAX_QUERY) {
        long v = strtol(tok, NULL, 0);
        if (v < 0) { fprintf(stderr, "Invalid page number: %s\n", tok); exit(-1); }
        buf[i++] = (uint8_t)v;
        tok = strtok(NULL, " ");
    }
    return i;
}

/**
 * CLI arguments parsing.
 * @param argc: argument count
 * @param argv: argument vector
 */
static args parseArgs(int argc, char *argv[]) {
    args ret;
    ret.num_frames = 3;
    ret.query_len  = 0;
    strcpy(ret.algo, "all");

    if (argc == 1) { printf(HELPMSG); exit(-1); }

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) { printf(HELPMSG); exit(0); }
        else if (!strcmp(argv[i], "-N") && i + 1 < argc) {
            ret.num_frames = (uint8_t)atoi(argv[++i]);
            if (ret.num_frames < 1) { fprintf(stderr, "N must be >= 1\n"); exit(-1); }
        }
        else if (!strcmp(argv[i], "-query") && i + 1 < argc)
            ret.query_len = parse_query_str(ret.query, argv[++i]);
        else if (!strcmp(argv[i], "-f") && i + 1 < argc && ret.query_len == 0)
            ret.query_len = read_file(ret.query, argv[++i]);
        else if (!strcmp(argv[i], "-a") && i + 1 < argc)
            strncpy(ret.algo, argv[++i], sizeof(ret.algo) - 1);
    }

    if (ret.query_len == 0) {
        fprintf(stderr, "Error: page reference sequence required (-query or -f).\n");
        exit(-1);
    }
    return ret;
}

int main(int argc, char *argv[]) {
    args a = parseArgs(argc, argv);

    printf("Frames: %d | Sequence length: %d | Algorithm: %s\n",
           a.num_frames, a.query_len, a.algo);

    logger_init(a.num_frames, a.query, a.query_len);
    logger_run(a.algo);
    logger_free();
    return 0;
}
