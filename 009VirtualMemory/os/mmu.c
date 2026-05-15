#include "mmu.h"
#include <string.h>
#include "RAM.h"
#include "page_table.h"
#include "random.h"

// page table
pt* table;

/**
 * Get the number of physical frames
 * @return the number of physical frames
 */
uint8_t get_num_frames(void) {
    return NUM_FRAMES;
}

/**
 * Initialization of the RAM. Sets each frame as FREE first, then proceeds to set a random amount to OCCUPIED.
 * @param num_vpage: number of virtual pages
 * @param occ_count: the number of frames to occupy
 */
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

/**
 * Free the RAM space.
 */
void ram_free(void) {
    pt_destroy(table);
}

/**
 * Get a copy of the current view of the RAM.
 * @param buffer: destination pointer
 */
void get_RAM_view(frame* buffer) {
    memcpy(buffer, frames, NUM_FRAMES * sizeof(frame));
}

/**
 * Find a FREE frame and returns its index.
 * @return a FREE frame's index or -1 if there's no more available frames
 */
int8_t allocate_frame(void) {
    for (int8_t i = 0; i < NUM_FRAMES; i++) {
        if (!frames[i].state) return i;
    }
    return -1;
}

/**
 * Loads the process into the RAM. If there are not enought frames to allocate, exits with error.
 * @param num_vpage: number of virtual pages of the process
 */
void load_process(uint8_t num_vpage) {
    int8_t free_idx;
    for (uint8_t i = 0; i < num_vpage; i++) {
        free_idx = allocate_frame();
        if (free_idx != -1) {
            pt_set_entry(table, i, free_idx);
            frames[free_idx].state = OCCUPIED;
            frames[free_idx].v_page = i;
        }
        else { 
            printf("ERROR! No more FREE frames to allocate. Exiting...\n");
            exit(-1);
        }  
    }
}

/**
 * Translation of virtual address to physical address.
 * @param v_addr: virtual address
 */
int16_t translate(uint16_t v_addr) {
    uint8_t vpn = (v_addr >> 8) & 0xFF;
    int16_t pfn = (int16_t)pt_get_pfn(table, vpn);
    if (pfn == -1) return -1;   // vpn greater than table capacity
    return (pfn << 8) + (v_addr & 0xFF);
}

/**
 * Count the occupied frames in the RAM.
 * @return the count of occupied frames
 */
uint8_t occupied_count(void) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_FRAMES; i++) {
        if (frames[i].state) count++;
    }
    return count;
}