#include "scheduler.h"
#include <string.h>

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

static void copy_threads(thread_t *dst, const thread_t *src, int n) {
    memcpy(dst, src, n * sizeof(thread_t));
    for (int i = 0; i < n; i++) {
        dst[i].remaining  = src[i].burst;
        dst[i].start      = -1;
        dst[i].finish     = 0;
        dst[i].waiting    = 0;
        dst[i].turnaround = 0;
    }
}

/* ── FIFO ─────────────────────────────────────────────────────────────────── */

/**
 * First-In First-Out scheduler.
 * Processes run in order of arrival, completing fully before the next starts.
 */
void scheduler_fifo(thread_t *threads, int n, stats_t *stats, gantt_t *gantt) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0;

    /* sort by arrival */
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (t[j].arrival < t[i].arrival) {
                thread_t tmp = t[i]; t[i] = t[j]; t[j] = tmp;
            }

    int time = 0;
    for (int i = 0; i < n; i++) {
        if (time < t[i].arrival) time = t[i].arrival;
        t[i].start      = time;
        t[i].waiting    = time - t[i].arrival;
        time           += t[i].burst;
        t[i].finish     = time;
        t[i].turnaround = t[i].finish - t[i].arrival;
        gantt_push(gantt, t[i].id, t[i].start, t[i].finish);
    }
    compute_stats(t, n, stats);
    memcpy(threads, t, n * sizeof(thread_t));
}

/* ── Round Robin ─────────────────────────────────────────────────────────── */

/**
 * Round Robin scheduler.
 * Processes run with a fixed time quantum, cycling through the ready queue.
 */
void scheduler_rr(thread_t *threads, int n, stats_t *stats, gantt_t *gantt) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0;

    /* sort by arrival */
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (t[j].arrival < t[i].arrival) {
                thread_t tmp = t[i]; t[i] = t[j]; t[j] = tmp;
            }

    int queue[MAX_THREADS * 100];
    int qhead = 0, qtail = 0;
    int done = 0, time = 0, next_arrival = 0;

    /* seed first arrivals */
    while (next_arrival < n && t[next_arrival].arrival <= time)
        { queue[qtail++ % (MAX_THREADS*100)] = next_arrival++; }
    if (qtail == qhead && next_arrival < n) {
        time = t[next_arrival].arrival;
        queue[qtail++ % (MAX_THREADS*100)] = next_arrival++;
    }

    while (done < n) {
        if (qhead == qtail) {
            /* CPU idle */
            time = t[next_arrival].arrival;
            while (next_arrival < n && t[next_arrival].arrival <= time)
                queue[qtail++ % (MAX_THREADS*100)] = next_arrival++;
        }
        int i = queue[qhead++ % (MAX_THREADS*100)];

        if (t[i].start == -1) t[i].start = time;

        int run = t[i].remaining < RR_QUANTUM ? t[i].remaining : RR_QUANTUM;
        gantt_push(gantt, t[i].id, time, time + run);
        time            += run;
        t[i].remaining  -= run;

        /* enqueue newly arrived threads */
        while (next_arrival < n && t[next_arrival].arrival <= time) {
            queue[qtail++ % (MAX_THREADS*100)] = next_arrival;
            next_arrival++;
        }

        if (t[i].remaining > 0) {
            queue[qtail++ % (MAX_THREADS*100)] = i;
        } else {
            t[i].finish     = time;
            t[i].turnaround = t[i].finish - t[i].arrival;
            t[i].waiting    = t[i].turnaround - t[i].burst;
            done++;
        }
    }
    compute_stats(t, n, stats);
    memcpy(threads, t, n * sizeof(thread_t));
}

/* ── SJF (non-preemptive) ────────────────────────────────────────────────── */

/**
 * Shortest Job First scheduler (non-preemptive).
 * Selects the process with the shortest burst from those that have arrived.
 */
void scheduler_sjf(thread_t *threads, int n, stats_t *stats, gantt_t *gantt) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0;

    int done_flag[MAX_THREADS] = {0};
    int done = 0, time = 0;

    while (done < n) {
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (done_flag[i] || t[i].arrival > time) continue;
            if (sel == -1 || t[i].burst < t[sel].burst) sel = i;
        }
        if (sel == -1) {
            /* idle — advance to next arrival */
            int nxt = -1;
            for (int i = 0; i < n; i++)
                if (!done_flag[i] && (nxt == -1 || t[i].arrival < t[nxt].arrival)) nxt = i;
            time = t[nxt].arrival;
            continue;
        }
        t[sel].start      = time;
        t[sel].waiting    = time - t[sel].arrival;
        time             += t[sel].burst;
        t[sel].finish     = time;
        t[sel].turnaround = t[sel].finish - t[sel].arrival;
        gantt_push(gantt, t[sel].id, t[sel].start, t[sel].finish);
        done_flag[sel] = 1;
        done++;
    }
    compute_stats(t, n, stats);
    memcpy(threads, t, n * sizeof(thread_t));
}

/* ── SRTF (preemptive SJF) ───────────────────────────────────────────────── */

/**
 * Shortest Remaining Time First scheduler (preemptive).
 * At each tick, selects the process with the shortest remaining time.
 */
void scheduler_srtf(thread_t *threads, int n, stats_t *stats, gantt_t *gantt) {
    thread_t t[MAX_THREADS];
    copy_threads(t, threads, n);
    gantt->count = 0;

    int done_flag[MAX_THREADS] = {0};
    int done = 0, time = 0;

    /* find total time span */
    int total = 0;
    for (int i = 0; i < n; i++) total += t[i].burst;
    int max_time = t[0].arrival;
    for (int i = 1; i < n; i++) if (t[i].arrival > max_time) max_time = t[i].arrival;
    max_time += total + 1;

    while (done < n && time <= max_time) {
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (done_flag[i] || t[i].arrival > time) continue;
            if (sel == -1 || t[i].remaining < t[sel].remaining) sel = i;
        }
        if (sel == -1) { time++; continue; }

        if (t[sel].start == -1) t[sel].start = time;
        gantt_push(gantt, t[sel].id, time, time + 1);
        t[sel].remaining--;
        time++;

        if (t[sel].remaining == 0) {
            t[sel].finish     = time;
            t[sel].turnaround = t[sel].finish - t[sel].arrival;
            t[sel].waiting    = t[sel].turnaround - t[sel].burst;
            done_flag[sel]    = 1;
            done++;
        }
    }
    compute_stats(t, n, stats);
    memcpy(threads, t, n * sizeof(thread_t));
}
