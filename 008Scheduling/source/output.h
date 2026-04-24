#ifndef OUTPUT_H
#define OUTPUT_H

#include "scheduler.h"

/* ANSI color codes per thread (cycles through 6 colors) */
#define COLOR_RESET  "\033[0m"

void output_print_thread_list  (thread_t *threads, int n);
void output_print_log          (const char *algo, event_log_t *elog);
void output_print_gantt        (const char *algo, gantt_t *gantt, int n_threads, thread_t *threads);
void output_print_stats        (const char *algo, stats_t *stats, thread_t *threads);
void output_write_log          (const char *path, const char *algo, event_log_t *elog);
void output_write_stats        (const char *path, const char *algo, stats_t *stats, thread_t *threads);

#endif /* OUTPUT_H */