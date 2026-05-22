#include "scheduler.h"
#include "RAM.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t hits   = 0;
uint32_t misses = 0;

static uint8_t  *ref_seq   = NULL;
static uint16_t  ref_len   = 0;

/* ── FIFO queue ───────────────────────────────────────────────────────────── */
static uint8_t fifo_queue[256];
static uint8_t fifo_head, fifo_count;

/* ── LRU linked list node ─────────────────────────────────────────────────── */
typedef struct lru_node {
    uint8_t          page;
    uint8_t          frame_idx;
    struct lru_node *prev;
    struct lru_node *next;
} lru_node;

static lru_node  lru_pool[256];
static lru_node *lru_head;   /* most recently used */
static lru_node *lru_tail;   /* least recently used */
static uint8_t   lru_count;

/* ── Second Chance structures ─────────────────────────────────────────────── */
typedef struct sc_node {
    uint8_t          page;
    uint8_t          frame_idx;
    uint8_t          ref_bit;
    struct sc_node  *prev;
    struct sc_node  *next;
} sc_node;

static sc_node  sc_pool[256];
static sc_node *sc_head;
static sc_node *sc_tail;
static uint8_t  sc_count;

/* ── helpers ─────────────────────────────────────────────────────────────── */

static int8_t find_page(uint8_t page) {
    for (uint8_t i = 0; i < num_frames; i++)
        if (frames[i] == page) return (int8_t)i;
    return -1;
}

static int8_t find_empty(void) {
    for (uint8_t i = 0; i < num_frames; i++)
        if (frames[i] == FRAME_EMPTY) return (int8_t)i;
    return -1;
}

/* ── init / reset ─────────────────────────────────────────────────────────── */

/**
 * Stores the reference sequence for MIN lookahead.
 * @param query: page reference array
 * @param len:   number of references
 */
void scheduler_init(uint8_t *query, uint16_t len) {
    ref_seq = query;
    ref_len = len;
}

/**
 * Resets all scheduler state and counters. Call between algorithms.
 */
void scheduler_reset(void) {
    hits = misses = 0;
    memset(frames, FRAME_EMPTY, num_frames * sizeof(frame));

    /* FIFO */
    memset(fifo_queue, FRAME_EMPTY, sizeof(fifo_queue));
    fifo_head  = 0;
    fifo_count = 0;

    /* LRU */
    lru_head = lru_tail = NULL;
    lru_count = 0;

    /* Second Chance */
    sc_head = sc_tail = NULL;
    sc_count = 0;
}

/* ── FIFO ─────────────────────────────────────────────────────────────────── */

/**
 * One FIFO access step.
 * @param page: referenced page number
 * @param step: current step index (unused, for signature consistency)
 */
access_result scheduler_fifo(uint8_t page, uint16_t step) {
    (void)step;
    access_result r = { 0, FRAME_EMPTY, page, 0 };

    int8_t idx = find_page(page);
    if (idx != -1) {
        hits++;
        r.hit       = 1;
        r.frame_idx = (uint8_t)idx;
        return r;
    }
    misses++;

    int8_t free_idx = find_empty();
    if (free_idx != -1) {
        frames[free_idx] = page;
        fifo_queue[(fifo_head + fifo_count) % num_frames] = (uint8_t)free_idx;
        fifo_count++;
        r.frame_idx = (uint8_t)free_idx;
    } else {
        uint8_t victim_fi = fifo_queue[fifo_head];
        r.evicted   = frames[victim_fi];
        r.frame_idx = victim_fi;
        frames[victim_fi] = page;
        fifo_queue[fifo_head] = victim_fi;
        fifo_head = (fifo_head + 1) % num_frames;
    }
    return r;
}

/* ── MIN / Optimal ────────────────────────────────────────────────────────── */

/**
 * One MIN access step. Evicts the page used farthest in the future.
 * Tie-break: smallest page id if multiple never appear again or share same future index.
 * @param page: referenced page number
 * @param step: current step index for lookahead
 */
access_result scheduler_min(uint8_t page, uint16_t step) {
    access_result r = { 0, FRAME_EMPTY, page, 0 };

    int8_t idx = find_page(page);
    if (idx != -1) {
        hits++;
        r.hit       = 1;
        r.frame_idx = (uint8_t)idx;
        return r;
    }
    misses++;

    int8_t free_idx = find_empty();
    if (free_idx != -1) {
        frames[free_idx] = page;
        r.frame_idx = (uint8_t)free_idx;
        return r;
    }

    /* find next use of each resident page */
    int8_t  victim_fi   = 0;
    int16_t farthest    = -1;
    uint8_t victim_page = frames[0];

    for (uint8_t i = 0; i < num_frames; i++) {
        uint8_t  p        = frames[i];
        int16_t  next_use = -1;
        for (uint16_t j = step + 1; j < ref_len; j++) {
            if (ref_seq[j] == p) { next_use = (int16_t)j; break; }
        }
        /* -1 means never used again → immediately best candidate */
        int16_t key = (next_use == -1) ? 30000 : next_use;
        if (key > farthest || (key == farthest && p < victim_page)) {
            farthest    = key;
            victim_fi   = (int8_t)i;
            victim_page = p;
        }
    }

    r.evicted         = frames[victim_fi];
    r.frame_idx       = (uint8_t)victim_fi;
    frames[victim_fi] = page;
    return r;
}

/* ── LRU linked list helpers ─────────────────────────────────────────────── */

static void lru_remove(lru_node *n) {
    if (n->prev) n->prev->next = n->next; else lru_head = n->next;
    if (n->next) n->next->prev = n->prev; else lru_tail = n->prev;
    n->prev = n->next = NULL;
}

static void lru_push_front(lru_node *n) {
    n->next = lru_head;
    n->prev = NULL;
    if (lru_head) lru_head->prev = n;
    lru_head = n;
    if (!lru_tail) lru_tail = n;
}

/* ── LRU ──────────────────────────────────────────────────────────────────── */

/**
 * One LRU access step. Evicts the least recently used page.
 * Tie-break: smallest page id.
 * @param page: referenced page number
 * @param step: unused
 */
access_result scheduler_lru(uint8_t page, uint16_t step) {
    (void)step;
    access_result r = { 0, FRAME_EMPTY, page, 0 };

    /* search existing node */
    for (lru_node *n = lru_head; n; n = n->next) {
        if (n->page == page) {
            hits++;
            r.hit       = 1;
            r.frame_idx = n->frame_idx;
            lru_remove(n);
            lru_push_front(n);
            return r;
        }
    }
    misses++;

    int8_t free_idx = find_empty();
    if (free_idx != -1) {
        frames[free_idx] = page;
        lru_node *n  = &lru_pool[lru_count++];
        n->page      = page;
        n->frame_idx = (uint8_t)free_idx;
        lru_push_front(n);
        r.frame_idx  = (uint8_t)free_idx;
        return r;
    }

    /* evict LRU (tail); tie-break smallest page id handled by always pushing
       new/reused to front — tail is the least recently used */
    lru_node *victim = lru_tail;
    r.evicted        = victim->page;
    r.frame_idx      = victim->frame_idx;
    frames[victim->frame_idx] = page;
    lru_remove(victim);
    victim->page = page;
    lru_push_front(victim);
    return r;
}

/* ── Second Chance ────────────────────────────────────────────────────────── */

static void sc_remove(sc_node *n) {
    if (n->prev) n->prev->next = n->next; else sc_head = n->next;
    if (n->next) n->next->prev = n->prev; else sc_tail = n->prev;
    n->prev = n->next = NULL;
}

static void sc_push_back(sc_node *n) {
    n->prev = sc_tail;
    n->next = NULL;
    if (sc_tail) sc_tail->next = n;
    sc_tail = n;
    if (!sc_head) sc_head = n;
}

/**
 * One Second Chance access step.
 * Uses a FIFO queue with a reference bit; on eviction gives pages one extra chance.
 * @param page: referenced page number
 * @param step: unused
 */
access_result scheduler_second_chance(uint8_t page, uint16_t step) {
    (void)step;
    access_result r = { 0, FRAME_EMPTY, page, 0 };

    for (sc_node *n = sc_head; n; n = n->next) {
        if (n->page == page) {
            hits++;
            n->ref_bit  = 1;
            r.hit       = 1;
            r.frame_idx = n->frame_idx;
            return r;
        }
    }
    misses++;

    int8_t free_idx = find_empty();
    if (free_idx != -1) {
        frames[free_idx] = page;
        sc_node *n   = &sc_pool[sc_count++];
        n->page      = page;
        n->frame_idx = (uint8_t)free_idx;
        n->ref_bit   = 0;
        sc_push_back(n);
        r.frame_idx  = (uint8_t)free_idx;
        return r;
    }

    /* clock sweep: skip nodes with ref_bit=1, clear their bit */
    while (sc_head->ref_bit) {
        sc_node *n = sc_head;
        n->ref_bit = 0;
        sc_remove(n);
        sc_push_back(n);
    }

    sc_node *victim   = sc_head;
    r.evicted         = victim->page;
    r.frame_idx       = victim->frame_idx;
    frames[victim->frame_idx] = page;
    sc_remove(victim);
    victim->page    = page;
    victim->ref_bit = 0;
    sc_push_back(victim);
    return r;
}
