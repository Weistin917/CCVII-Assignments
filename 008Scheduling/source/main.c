#include <stdio.h>
#include <string.h>
#include <time.h>

#include "scheduler.h"
#include "random.h"
#include "output.h"

#define LOG_FILE    "008Scheduling/logs/run.log"
#define STATS_FILE  "008Scheduling/logs/stats.log"

static void run_algo(const char *name,
                     void (*sched)(thread_t *, int, stats_t *, gantt_t *, event_log_t *),
                     thread_t *orig, int n) {
    thread_t  threads[MAX_THREADS];
    stats_t   stats;
    gantt_t   gantt;
    event_log_t elog;

    memcpy(threads, orig, n * sizeof(thread_t));
    gantt.count = 0;
    elog.count = 0;

    sched(threads, n, &stats, &gantt, &elog);

    output_print_log(name, &elog);
    output_print_gantt(name, &gantt, n, threads);
    output_print_stats(name, &stats, threads);
    
    output_write_log(LOG_FILE, name, &elog);
    output_write_stats(STATS_FILE, name, &stats, threads);
}

int main(void) {
    rand_seed((unsigned int)time(NULL));

    int n = rand_range(MIN_THREADS, MAX_THREADS);
    thread_t orig[MAX_THREADS];

    for (int i = 0; i < n; i++) {
        orig[i].id        = i;
        orig[i].burst     = rand_range(1, MAX_BURST);
        orig[i].arrival   = rand_range(0, MAX_ARRIVAL);
        orig[i].remaining = orig[i].burst;
        orig[i].start     = -1;
        orig[i].finish    = 0;
        orig[i].waiting   = 0;
        orig[i].turnaround= 0;
        orig[i].color     = i;
    }

    /* clear log files */
    FILE *f1 = fopen(LOG_FILE,   "w");
    if(f1) fclose(f1);
    FILE *f2 = fopen(STATS_FILE, "w");
    if(f2) fclose(f2);

    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║         CPU Scheduling Simulation                ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    printf("  Threads: %d  |  RR Quantum: %ds\n", n, RR_QUANTUM);
    output_print_thread_list(orig, n);

    run_algo("FIFO Scheduling", scheduler_fifo, orig, n);
    run_algo("Round Robin Scheduling (Quantum 2)", scheduler_rr, orig, n);
    run_algo("SJF Scheduling", scheduler_sjf, orig, n);
    run_algo("SRTF Scheduling", scheduler_srtf, orig, n);

    return 0;
}