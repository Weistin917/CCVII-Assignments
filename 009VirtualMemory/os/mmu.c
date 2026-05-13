/**
 * Memory allocator
 * * Page table
 * * ram_init()
 * * allocate_frame()
 * * load_process()
 * * translate()
 */
#include "mmu.h"
#include <string.h>
#include "RAM.h"
#include "page_table.h"
#include "random.h"

pt* table;

uint8_t get_num_frames(void) {
    return NUM_FRAMES;
}

void ram_init(uint8_t num_vpage, uint8_t occ_count) {
    table = pt_create(num_vpage);

    for (uint8_t i = 0; i < NUM_FRAMES; i++) {
        frames[i].state = FREE;
        frames[i].v_page = 0;
    }
    
    int rand_idx;
    for (uint8_t i = 0; i < occ_count; i++) {
        while (1) {
            rand_idx = rand_range(0, NUM_FRAMES - 1);
            
            if (!frames[rand_idx].state) {
                frames[rand_idx].state = PREOCCUPIED;
                break;
            }
        }
    }
}

void get_RAM_view(frame* buffer) {
    memcpy(buffer, frames, NUM_FRAMES * sizeof(frame));
}

int8_t allocate_frame(void) {
    for (int8_t i = 0; i < NUM_FRAMES; i++) {
        if (!frames[i].state) return i;
    }
    return -1;
}

void load_process(uint8_t num_vpage) {
    int8_t free_idx;
    for (uint8_t i = 0; i < num_vpage; i++) {
        free_idx = allocate_frame();
        if (free_idx != -1) {
            pt_set_entry(table, i, free_idx);
            frames[free_idx].state = OCCUPIED;
            frames[free_idx].v_page = i;
        }
        else { } // show error message       
    }
}

int16_t translate(uint16_t v_addr) {
    uint8_t vpn = (v_addr >> 8) & 0xFF;
    int16_t pfn = (int16_t)pt_get_pfn(table, vpn);
    if (pfn == -1) return -1;   // vpn greater than table capacity
    return (pfn << 8) + (v_addr & 0xFF);
}

uint8_t occupied_count(void) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_FRAMES; i++) {
        if (frames[i].state) count++;
    }
    return count;
}