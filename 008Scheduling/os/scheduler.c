#include "scheduler.h"
#include <string.h>
#include <stdio.h>

/* ── helpers ─────────────────────────────────────────────────────────────── */
static void compute_stats(thread_t *threads, int n, stats_t *stats) {
    stats->n = n;
    double sw = 0, st = 0;
    for (int i = 0; i < n; i++) {
        stats->waiting[i]    = threads[i].waiting;
        stats->turnaround[i] = threads[i].turnaround;
        sw += threads[i].waiting;
        st += threads[i].turnaround;
    }
    stats->avg_waiting    = sw / n;
    stats->avg_turnaround = st / n;
}

static void gantt_push(gantt_t *g, int tid, int start, int end) {
    if (g->count > 0 && g->slots[g->count - 1].thread_id == tid) {
        g->slots[g->count - 1].end = end;
        return;
    }
    g->slots[g->count].thread_id = tid;
    g->slots[g->count].start     = start;
    g->slots[g->count].end       = end;
    g->count++;
}

static void log_event(event_log_t *elog, int time, int tid, const char *msg) {
    elog->events[elog->count].time = time;
    elog->events[elog->count].thread_id = tid;
    strncpy(elog->events[elog->count].message, msg, 63);
    elog->count++;
}

static void copy_threads(thread_t *dst, const thread_t *src, int n) {
    memcpy(dst, src, n * sizeof(thread_t));
    for (int i = 0; i < n; i++) {
        dst[i].remaining = src[i].burst;
        dst[i].start = -1;
    }
}

// Helper to log arrivals 
static void check_arrivals(thread_t *threads, int n, int prev_time, int current_time, event_log_t *elog) {
    for (int t = prev_time; t <= current_time; t++) {
        for (int i = 0; i < n; i++) {
            if (threads[i].arrival == t) {
                char msg[64];
                sprintf(msg, "Process %d (Burst %d): Arrived", threads[i].id, threads[i].burst);
                log_event(elog, t, threads[i].id, msg);
            }
        }
    }
}

/* ── FIFO ────────────────────────────────────────────────────────────────── */
void scheduler_fifo(thread_t *threads, int n, stats_t *stats, gantt_t *gantt, event_log_t *elog) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0; elog->count = 0;

    int done_flag[MAX_THREADS] = {0};
    int done = 0, time = 0;

    while (done < n) {
        int sel = -1;
        int min_arr = 999999;
        for (int i = 0; i < n; i++) {
            if (!done_flag[i] && t[i].arrival <= time) {
                if (t[i].arrival < min_arr) {
                    min_arr = t[i].arrival;
                    sel = i;
                }
            }
        }

        if (sel == -1) {
            check_arrivals(t, n, time, time, elog);
            gantt_push(gantt, -1, time, time + 1);
            time++;
        } else {
            if (t[sel].start == -1) {
                t[sel].start = time;
                t[sel].waiting = time - t[sel].arrival;
                char msg[64];
                sprintf(msg, "Process %d (Burst %d): Started (waited %d seconds)", t[sel].id, t[sel].burst, t[sel].waiting);
                log_event(elog, time, t[sel].id, msg);
            }
            int start_time = time;
            time += t[sel].remaining;
            t[sel].remaining = 0;
            gantt_push(gantt, t[sel].id, start_time, time);
            
            check_arrivals(t, n, start_time, time - 1, elog);
            
            t[sel].finish = time;
            t[sel].turnaround = t[sel].finish - t[sel].arrival;
            done_flag[sel] = 1; done++;

            char msg[64];
            sprintf(msg, "Process %d: Completed", t[sel].id);
            log_event(elog, time, t[sel].id, msg);
            
            threads[sel] = t[sel];
        }
    }
    check_arrivals(t, n, time, time, elog);
    compute_stats(threads, n, stats);
}

/* ── RR ──────────────────────────────────────────────────────────────────── */
void scheduler_rr(thread_t *threads, int n, stats_t *stats, gantt_t *gantt, event_log_t *elog) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0; elog->count = 0;

    int time = 0, done = 0;
    int q[1000], head = 0, tail = 0;
    int in_q[MAX_THREADS] = {0};

    while (done < n) {
        check_arrivals(t, n, time, time, elog);
        for (int i = 0; i < n; i++) {
            if (t[i].arrival <= time && t[i].remaining > 0 && !in_q[i]) {
                q[tail++] = i;
                in_q[i] = 1;
            }
        }

        if (head < tail) {
            int sel = q[head++];
            in_q[sel] = 0; 

            if (t[sel].start == -1) {
                t[sel].start = time;
                char msg[64];
                sprintf(msg, "Process %d (Burst %d): Started", t[sel].id, t[sel].burst);
                log_event(elog, time, t[sel].id, msg);
            }

            int run_time = (t[sel].remaining < RR_QUANTUM) ? t[sel].remaining : RR_QUANTUM;
            gantt_push(gantt, t[sel].id, time, time + run_time);
            
            for(int step = 1; step <= run_time; step++) {
                time++;
                check_arrivals(t, n, time, time, elog);
                for (int i = 0; i < n; i++) {
                    if (t[i].arrival == time && t[i].remaining > 0 && !in_q[i] && i != sel) {
                        q[tail++] = i;
                        in_q[i] = 1;
                    }
                }
            }

            t[sel].remaining -= run_time;

            if (t[sel].remaining > 0) {
                char msg[64];
                // Fulfills Preemption Requirement exactly as requested
                sprintf(msg, "Process %d (Burst %d remaining): Preempted", t[sel].id, t[sel].remaining);
                log_event(elog, time, t[sel].id, msg);
                q[tail++] = sel;
                in_q[sel] = 1;
            } else {
                t[sel].finish = time;
                t[sel].turnaround = t[sel].finish - t[sel].arrival;
                t[sel].waiting = t[sel].turnaround - t[sel].burst;
                done++;
                
                char msg[64];
                sprintf(msg, "Process %d: Completed", t[sel].id);
                log_event(elog, time, t[sel].id, msg);
                
                threads[sel] = t[sel];
            }
        } else {
            gantt_push(gantt, -1, time, time + 1);
            time++;
        }
    }
    compute_stats(threads, n, stats);
}

/* ── SJF ─────────────────────────────────────────────────────────────────── */
void scheduler_sjf(thread_t *threads, int n, stats_t *stats, gantt_t *gantt, event_log_t *elog) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0; elog->count = 0;

    int done_flag[MAX_THREADS] = {0};
    int done = 0, time = 0;

    while (done < n) {
        check_arrivals(t, n, time, time, elog);
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (!done_flag[i] && t[i].arrival <= time) {
                if (sel == -1 || t[i].burst < t[sel].burst) {
                    sel = i;
                }
            }
        }

        if (sel == -1) {
            gantt_push(gantt, -1, time, time + 1);
            time++;
        } else {
            if (t[sel].start == -1) {
                t[sel].start = time;
                t[sel].waiting = time - t[sel].arrival;
                char msg[64];
                sprintf(msg, "Process %d (Burst %d): Started (waited %d seconds)", t[sel].id, t[sel].burst, t[sel].waiting);
                log_event(elog, time, t[sel].id, msg);
            }
            int start_time = time;
            time += t[sel].remaining;
            t[sel].remaining = 0;
            gantt_push(gantt, t[sel].id, start_time, time);
            
            check_arrivals(t, n, start_time + 1, time - 1, elog);
            t[sel].finish = time;
            t[sel].turnaround = t[sel].finish - t[sel].arrival;
            done_flag[sel] = 1; done++;
            check_arrivals(t, n, time, time, elog);

            char msg[64];
            sprintf(msg, "Process %d: Completed", t[sel].id);
            log_event(elog, time, t[sel].id, msg);
            
            threads[sel] = t[sel];
        }
    }
    compute_stats(threads, n, stats);
}

/* ── SRTF ────────────────────────────────────────────────────────────────── */
void scheduler_srtf(thread_t *threads, int n, stats_t *stats, gantt_t *gantt, event_log_t *elog) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0; elog->count = 0;

    int done_flag[MAX_THREADS] = {0};
    int done = 0, time = 0;
    int last_running = -1;

    while (done < n) {
        check_arrivals(t, n, time, time, elog);
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (!done_flag[i] && t[i].arrival <= time) {
                if (sel == -1 || t[i].remaining < t[sel].remaining) {
                    sel = i;
                }
            }
        }

        // Fulfills Preemption Requirement exactly as requested
        if (last_running != -1 && last_running != sel && t[last_running].remaining > 0) {
            char msg[64];
            sprintf(msg, "Process %d (Burst %d remaining): Preempted", t[last_running].id, t[last_running].remaining);
            log_event(elog, time, t[last_running].id, msg);
        }

        if (sel == -1) {
            gantt_push(gantt, -1, time, time + 1);
            time++;
            last_running = -1;
        } else {
            if (t[sel].start == -1 || last_running != sel) {
                if (t[sel].start == -1) t[sel].start = time;
                char msg[64];
                sprintf(msg, "Process %d (Burst %d): Started/Resumed", t[sel].id, t[sel].burst);
                log_event(elog, time, t[sel].id, msg);
            }
            gantt_push(gantt, t[sel].id, time, time + 1);
            t[sel].remaining--;
            time++;

            if (t[sel].remaining == 0) {
                t[sel].finish = time;
                t[sel].turnaround = t[sel].finish - t[sel].arrival;
                t[sel].waiting = t[sel].turnaround - t[sel].burst;
                done_flag[sel] = 1; done++;
                
                char msg[64];
                sprintf(msg, "Process %d: Completed", t[sel].id);
                log_event(elog, time, t[sel].id, msg);
                threads[sel] = t[sel];
            }
            last_running = sel;
        }
    }
    compute_stats(threads, n, stats);
}