#include "output.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

static const char *thread_color(int id) { return COLORS[id % N_COLORS]; }

static void timestamp_str(int sim_time, char *buf, size_t len) {
    time_t base = 1742554800; /* Fri Mar 21 15:00:00 2025 */
    base += sim_time;
    struct tm *tm_info = localtime(&base);
    strftime(buf, len, "[%a %b %d %H:%M:%S %Y]", tm_info);
}

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

/* ── Logging ─────────────────────────────────────────────────────────────── */

void output_print_log(const char *algo, event_log_t *elog) {
    printf("\n--- %s ---\n\n", algo);
    char tbuf[64];
    for (int i = 0; i < elog->count; i++) {
        timestamp_str(elog->events[i].time, tbuf, sizeof(tbuf));
        printf("%s %s\n", tbuf, elog->events[i].message);
    }
}

void output_write_log(const char *path, const char *algo, event_log_t *elog) {
    FILE *f = fopen(path, "a");
    if (!f) return;
    fprintf(f, "\n--- %s ---\n\n", algo);
    char tbuf[64];
    for (int i = 0; i < elog->count; i++) {
        timestamp_str(elog->events[i].time, tbuf, sizeof(tbuf));
        fprintf(f, "%s %s\n", tbuf, elog->events[i].message);
    }
    fclose(f);
}

/* ── Gantt Chart (Real-Time Animated) ────────────────────────────────────── */

void output_print_gantt(const char *algo, gantt_t *gantt, int n_threads, thread_t *threads) {
    int total_time = 0;
    if (gantt->count > 0) total_time = gantt->slots[gantt->count - 1].end;
    
    // Print Dynamic Top Border
    printf("\n  /");
    for(int i = 0; i < total_time; i++) printf("-");
    printf("\\\n  |");
    
    // Print Inner Execution Bar in Real-Time
    for (int i = 0; i < gantt->count; i++) {
        int dur = gantt->slots[i].end - gantt->slots[i].start;
        for (int t = 0; t < dur; t++) {
            if (gantt->slots[i].thread_id == -1) {
                printf(".");
            } else {
                printf("%s|%s", thread_color(threads[gantt->slots[i].thread_id].color), COLOR_RESET);
            }
            fflush(stdout);   // Force printing to the screen immediately
            usleep(150000);   // Simulate processing (150ms delay per burst unit)
        }
    }
    
    // Print Dynamic Bottom Border
    printf("|\n  \\");
    for(int i = 0; i < total_time; i++) printf("-");
    printf("/,,, %s\n\n", algo);
}

/* ── Stats ───────────────────────────────────────────────────────────────── */

void output_print_stats(const char *algo, stats_t *stats, thread_t *threads) {
    printf("Avg Waiting Time: %.2f seconds\n", stats->avg_waiting);
    printf("Avg Turnaround Time: %.2f seconds\n\n", stats->avg_turnaround);
    (void)threads;
}

void output_write_stats(const char *path, const char *algo, stats_t *stats, thread_t *threads) {
    FILE *f = fopen(path, "a");
    if (!f) return;
    fprintf(f, "=== %s ===\n", algo);
    fprintf(f, "Avg Waiting Time: %.2f seconds\n", stats->avg_waiting);
    fprintf(f, "Avg Turnaround Time: %.2f seconds\n\n", stats->avg_turnaround);
    fclose(f);
}