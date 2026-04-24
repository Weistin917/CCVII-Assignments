#include "output.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── color helpers ────────────────────────────────────────────────────────── */

static const char *COLORS[] = {
    "\033[91m", /* bright red    */
    "\033[92m", /* bright green  */
    "\033[93m", /* bright yellow */
    "\033[94m", /* bright blue   */
    "\033[95m", /* bright magenta*/
    "\033[96m", /* bright cyan   */
    "\033[97m", /* bright white  */
};
#define N_COLORS 7

static const char *thread_color(int id) {
    return COLORS[id % N_COLORS];
}

static void timestamp_str(int sim_time, char *buf, size_t len) {
    /* Use a fixed base time + sim_time seconds for readable output */
    time_t base = 1742554800; /* Fri Mar 21 15:00:00 2025 */
    base += sim_time;
    struct tm *tm_info = localtime(&base);
    strftime(buf, len, "[%a %b %d %H:%M:%S %Y]", tm_info);
}

/* ── thread list ─────────────────────────────────────────────────────────── */

/**
 * Prints the list of threads with their color, arrival and burst time.
 * @param threads: array of threads
 * @param n: number of threads
 */
void output_print_thread_list(thread_t *threads, int n) {
    printf("\n┌─────────────────────────────────────────────────┐\n");
    printf("│  %-6s  %-6s  %-12s  %-12s      │\n",
           "ID", "Color", "Arrival(s)", "Burst(s)");
    printf("├─────────────────────────────────────────────────┤\n");
    for (int i = 0; i < n; i++) {
        printf("│  T%-5d  %s██%s     %-12d  %-12d      │\n",
               threads[i].id,
               thread_color(threads[i].id), COLOR_RESET,
               threads[i].arrival, threads[i].burst);
    }
    printf("└─────────────────────────────────────────────────┘\n\n");
}

/* ── log (terminal + file) ───────────────────────────────────────────────── */

/**
 * Formats a single log line for a completed thread.
 * @param buf: output buffer
 * @param len: buffer length
 * @param algo: algorithm name
 * @param t: scheduled thread
 * @param orig: original (pre-schedule) thread data
 */
static void format_log_lines(FILE *f, const char *algo,
                              thread_t *threads, int n, thread_t *orig) {
    char ts[64];
    fprintf(f, "--- %s Scheduling ---\n", algo);
    for (int i = 0; i < n; i++) {
        timestamp_str(threads[i].arrival, ts, sizeof(ts));
        fprintf(f, "%s Process %d (Burst %d): Arrived\n",
                ts, threads[i].id, orig[threads[i].id].burst);

        timestamp_str(threads[i].start, ts, sizeof(ts));
        if (threads[i].waiting > 0)
            fprintf(f, "%s Process %d (Burst %d): Arrived at %d, Started (waited %.2f seconds)\n",
                    ts, threads[i].id, orig[threads[i].id].burst,
                    threads[i].arrival, (double)threads[i].waiting);
        else
            fprintf(f, "%s Process %d (Burst %d): Started\n",
                    ts, threads[i].id, orig[threads[i].id].burst);

        timestamp_str(threads[i].finish, ts, sizeof(ts));
        fprintf(f, "%s Process %d (Burst %d): Completed\n",
                ts, threads[i].id, orig[threads[i].id].burst);
    }
    fprintf(f, "\n");
}

/**
 * Prints the execution log for one algorithm to stdout.
 */
void output_print_log(const char *algo, thread_t *threads, int n,
                      thread_t *orig, int n_orig) {
    (void)n_orig;
    format_log_lines(stdout, algo, threads, n, orig);
}

/**
 * Appends the execution log for one algorithm to a file.
 */
void output_write_log(const char *path, const char *algo,
                      thread_t *threads, int n, thread_t *orig) {
    FILE *f = fopen(path, "a");
    if (!f) return;
    format_log_lines(f, algo, threads, n, orig);
    fclose(f);
}

/* ── gantt chart ─────────────────────────────────────────────────────────── */

#define GANTT_WIDTH 60

/**
 * Renders a dynamic ASCII Gantt bar to the terminal.
 * Each slot is colored by thread id.
 * @param algo: algorithm name
 * @param gantt: completed gantt data
 * @param n_threads: number of threads
 * @param threads: thread array (for color mapping)
 */
void output_print_gantt(const char *algo, gantt_t *gantt,
                        int n_threads, thread_t *threads) {
    (void)n_threads;
    (void)threads;

    /* compute total simulation time */
    int total = 0;
    for (int i = 0; i < gantt->count; i++)
        if (gantt->slots[i].end > total) total = gantt->slots[i].end;

    /* build bar string — each unit = one cell, scaled to GANTT_WIDTH */
    char bar[GANTT_WIDTH + 1];
    int  bar_color[GANTT_WIDTH];
    memset(bar, '.', GANTT_WIDTH);
    bar[GANTT_WIDTH] = '\0';

    for (int i = 0; i < gantt->count; i++) {
        int s = gantt->slots[i].start * GANTT_WIDTH / (total ? total : 1);
        int e = gantt->slots[i].end   * GANTT_WIDTH / (total ? total : 1);
        if (e > GANTT_WIDTH) e = GANTT_WIDTH;
        for (int c = s; c < e; c++) {
            bar[c]       = '|';
            bar_color[c] = gantt->slots[i].thread_id;
        }
    }

    /* top border */
    printf("/");
    for (int i = 0; i < GANTT_WIDTH; i++) printf("-");
    printf("\\\n");

    /* bar */
    printf(" ");
    for (int i = 0; i < GANTT_WIDTH; i++) {
        if (bar[i] == '|')
            printf("%s|%s", thread_color(bar_color[i]), COLOR_RESET);
        else
            printf(".");
    }
    printf("\n");

    /* bottom border + label */
    printf("\\");
    for (int i = 0; i < GANTT_WIDTH; i++) printf("-");
    printf("/ %s\n\n", algo);
}

/* ── statistics ──────────────────────────────────────────────────────────── */

/**
 * Prints statistics for one algorithm to stdout.
 */
void output_print_stats(const char *algo, stats_t *stats, thread_t *threads) {
    printf("=== %s Statistics ===\n", algo);
    printf("Waiting Times:    [");
    for (int i = 0; i < stats->n; i++)
        printf("%.2f%s", stats->waiting[i], i < stats->n - 1 ? ", " : "");
    printf("]\n");
    printf("Avg Waiting Time: %.2f seconds\n", stats->avg_waiting);

    printf("Turnaround Times: [");
    for (int i = 0; i < stats->n; i++)
        printf("%.2f%s", stats->turnaround[i], i < stats->n - 1 ? ", " : "");
    printf("]\n");
    printf("Avg Turnaround:   %.2f seconds\n\n", stats->avg_turnaround);
    (void)threads;
}

/**
 * Appends statistics for one algorithm to a file.
 */
void output_write_stats(const char *path, const char *algo,
                        stats_t *stats, thread_t *threads) {
    FILE *f = fopen(path, "a");
    if (!f) return;
    fprintf(f, "=== %s ===\n", algo);
    fprintf(f, "Waiting Times:    [");
    for (int i = 0; i < stats->n; i++)
        fprintf(f, "%.2f%s", stats->waiting[i], i < stats->n - 1 ? ", " : "");
    fprintf(f, "]\nAvg Waiting Time: %.2f\n", stats->avg_waiting);
    fprintf(f, "Turnaround Times: [");
    for (int i = 0; i < stats->n; i++)
        fprintf(f, "%.2f%s", stats->turnaround[i], i < stats->n - 1 ? ", " : "");
    fprintf(f, "]\nAvg Turnaround:   %.2f\n\n", stats->avg_turnaround);
    fclose(f);
    (void)threads;
}
