#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define MAX_THREADS     15
#define MIN_THREADS     5
#define MAX_BURST       10
#define MAX_ARRIVAL     20
#define RR_QUANTUM      2

typedef struct {
    int     id;
    int     burst;
    int     arrival;
    int     remaining;
    int     start;
    int     finish;
    int     waiting;
    int     turnaround;
    int     color;      // ANSI color code index
} thread_t;

typedef struct {
    double  avg_waiting;
    double  avg_turnaround;
    double  waiting[MAX_THREADS];
    double  turnaround[MAX_THREADS];
    int     n;
} stats_t;

/* Gantt chart: sequence of (thread_id, duration) slots */
typedef struct {
    int thread_id;  // -1 = idle
    int start;
    int end;
} gantt_slot_t;

typedef struct {
    gantt_slot_t slots[1024];
    int          count;
} gantt_t;

void scheduler_fifo (thread_t *threads, int n, stats_t *stats, gantt_t *gantt);
void scheduler_rr   (thread_t *threads, int n, stats_t *stats, gantt_t *gantt);
void scheduler_sjf  (thread_t *threads, int n, stats_t *stats, gantt_t *gantt);
void scheduler_srtf (thread_t *threads, int n, stats_t *stats, gantt_t *gantt);

#endif /* SCHEDULER_H */
