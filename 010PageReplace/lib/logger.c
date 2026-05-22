#include "logger.h"
#include "../os/RAM.h"
#include "../os/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLR_RST     "\033[0m"
#define CLR_GREEN   "\033[92m"
#define CLR_RED     "\033[91m"
#define CLR_YELLOW  "\033[93m"
#define CLR_CYAN    "\033[96m"
#define CLR_BOLD    "\033[1m"

static uint8_t  *g_query = NULL;
static uint16_t  g_len   = 0;

/* ── disk ASCII art ──────────────────────────────────────────────────────── */

static const char *DISK_ART[] = {
    "   .---.   ",
    "  /     \\  ",
    " | (( )) | ",
    "  \\     /  ",
    "   '---'   ",
};
#define DISK_LINES 5

/* ── RAM view display ─────────────────────────────────────────────────────── */

static void print_ram(frame *view, uint8_t highlight, uint8_t highlight_fi,
                      const char *hl_color) {
    uint8_t cols = 10;
    uint8_t width = cols * 10 + 2;

    /* top border */
    printf("/");
    for (uint8_t i = 0; i < width; i++) printf("-");
    printf("\\\n");

    for (uint8_t i = 0; i < num_frames; i++) {
        if (i % cols == 0) printf("|");
        if (highlight && i == highlight_fi)
            printf("%s", hl_color);
        if (view[i] == FRAME_EMPTY)
            printf("|%03d:   |", i);
        else
            printf("|%03d:%03d|", i, view[i]);
        if (highlight && i == highlight_fi)
            printf("%s", CLR_RST);
        if (i % cols == cols - 1 || i == num_frames - 1)
            printf("|\n");
    }

    printf("\\");
    for (uint8_t i = 0; i < width; i++) printf("-");
    printf("/\n");
}

/* ── disk animation ───────────────────────────────────────────────────────── */

static void animate_arrow(int forward) {
    const char *seqs_fwd[] = { "-", "--", "---", "--->" };
    const char *seqs_bwd[] = { "-", "--", "---", "<---" };
    const char **seqs = forward ? seqs_fwd : seqs_bwd;
    for (int i = 0; i < 4; i++) {
        printf("\r   %s   ", seqs[i]);
        fflush(stdout);
        usleep(200000);
    }
    printf("\n");
}

static void show_disk_transfer(frame *before, frame *after,
                                uint8_t fi, uint8_t evicted, uint8_t loaded) {
    printf("\n" CLR_YELLOW "  [Page Fault — accessing disk...]" CLR_RST "\n\n");

    /* before row — evicted frame in red */
    printf("  Before:\n");
    printf("  ");
    for (uint8_t i = 0; i < num_frames && i < 4; i++) {
        if (i == fi) printf(CLR_RED);
        if (before[i] == FRAME_EMPTY) printf("|%03d:   |", i);
        else printf("|%03d:%03d|", i, before[i]);
        if (i == fi) printf(CLR_RST);
    }
    if (num_frames > 4) printf("...");
    printf("\n");

    /* arrow forward */
    printf("  ");
    animate_arrow(1);

    /* disk art */
    for (int i = 0; i < DISK_LINES; i++)
        printf("               %s\n", DISK_ART[i]);
    printf("  (evicted: %s%d%s  loaded: %s%d%s)\n",
           CLR_RED, evicted, CLR_RST, CLR_GREEN, loaded, CLR_RST);

    /* arrow backward */
    printf("  ");
    animate_arrow(0);

    /* after row — loaded frame in green */
    printf("  After:\n");
    printf("  ");
    for (uint8_t i = 0; i < num_frames && i < 4; i++) {
        if (i == fi) printf(CLR_GREEN);
        if (after[i] == FRAME_EMPTY) printf("|%03d:   |", i);
        else printf("|%03d:%03d|", i, after[i]);
        if (i == fi) printf(CLR_RST);
    }
    if (num_frames > 4) printf("...");
    printf("\n\n");
}

/* ── run one algorithm ────────────────────────────────────────────────────── */

typedef access_result (*algo_fn)(uint8_t, uint16_t);

static void run_one(const char *name, algo_fn fn) {
    scheduler_reset();

    printf("\n");
    printf(CLR_BOLD "══════════════════════════════════════\n");
    printf("  %s\n", name);
    printf("══════════════════════════════════════" CLR_RST "\n\n");

    frame before[256], after[256];

    for (uint16_t i = 0; i < g_len; i++) {
        uint8_t page = g_query[i];
        RAM_get_view(before);

        access_result r = fn(page, i);

        RAM_get_view(after);

        /* step header */
        printf("Step %3d | Page %3d | ", i + 1, page);
        if (r.hit) {
            printf(CLR_GREEN "HIT " CLR_RST "\n");
            print_ram(after, 1, r.frame_idx, CLR_GREEN);
        } else {
            printf(CLR_RED "MISS" CLR_RST);
            if (r.evicted != FRAME_EMPTY)
                printf("  (evicted: %d)", r.evicted);
            printf("\n");

            if (r.evicted != FRAME_EMPTY)
                show_disk_transfer(before, after, r.frame_idx, r.evicted, r.loaded);
            else
                print_ram(after, 1, r.frame_idx, CLR_GREEN);
        }

        printf("Press Enter for next step...");
        while (getchar() != '\n');
    }

    printf("\n" CLR_BOLD "Results — %s:" CLR_RST "\n", name);
    printf("  Hits:   %u\n", hits);
    printf("  Misses: %u\n", misses);
    printf("  Hit rate: %.2f%%\n\n",
           (hits + misses) ? 100.0 * hits / (hits + misses) : 0.0);
}

/* ── public API ───────────────────────────────────────────────────────────── */

/**
 * Initializes logger, RAM and scheduler with the given configuration.
 * @param nf:    number of physical frames
 * @param query: page reference sequence
 * @param len:   sequence length
 */
void logger_init(uint8_t nf, uint8_t *query, uint16_t len) {
    RAM_init(nf);
    scheduler_init(query, len);
    g_query = query;
    g_len   = len;
}

/**
 * Runs the specified scheduling algorithm with step-by-step display.
 * @param algo: "fifo" | "min" | "lru" | "second_chance" | "all"
 */
void logger_run(const char *algo) {
    int do_fifo = 0, do_min = 0, do_lru = 0, do_sc = 0;
    if (!strcmp(algo, "all"))           { do_fifo = do_min = do_lru = do_sc = 1; }
    else if (!strcmp(algo, "fifo"))     { do_fifo = 1; }
    else if (!strcmp(algo, "min"))      { do_min  = 1; }
    else if (!strcmp(algo, "lru"))      { do_lru  = 1; }
    else if (!strcmp(algo, "second_chance")) { do_sc = 1; }

    if (do_fifo) run_one("FIFO",          scheduler_fifo);
    if (do_min)  run_one("MIN (Optimal)", scheduler_min);
    if (do_lru)  run_one("LRU",           scheduler_lru);
    if (do_sc)   run_one("Second Chance", scheduler_second_chance);
}

/**
 * Frees RAM memory.
 */
void logger_free(void) {
    RAM_free();
}
