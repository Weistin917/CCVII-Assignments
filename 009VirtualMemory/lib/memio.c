/**
 * Memory input/output display
 * * RAM_init()
 * * RAM_map()
 * * summary()
 * * addr_translate()
 */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "memio.h"
#include "mmu.h"
#include "random.h"

void RAM_map(void);
void summary(void);

frame* current_ram_view;
uint8_t num_frames;

void RAM_init(uint8_t num_vpage) {
    num_frames = get_num_frames();
    
    uint8_t occ_count;
    while (1) {
        occ_count = (uint8_t)rand_range(num_frames / 10, num_frames * 3 / 5);
        if ((num_vpage > 10) && (num_frames - occ_count >= num_vpage)) break;
        if (num_frames - occ_count >= 10) break;
    }

    ram_init(num_vpage, occ_count);

    assert(num_frames - occupied_count() >= num_vpage);

    printf("Physical RAM (%d) after random init:\n", num_frames);
    memory_printout();
    
    load_process(num_vpage);

    printf("Physical RAM (%d) after loading processes:\n", num_frames);
    memory_printout();
}

void memory_printout(void) {
    get_RAM_view(current_ram_view);

    summary();
    RAM_map();
}

void RAM_map(void) {
    // print memory map with colors
    
}

void summary(void) {
    uint8_t occ = occupied_count();
    
    printf("Free Frames: %d || Occupied Frames: %d\n", num_frames - occ, occ);
}

void addr_translate(uint16_t v_addr) {
    int16_t p_addr = translate(v_addr);
    if (p_addr == -1) {
        printf("ERROR! VA out of range.\n");
        return;
    }
    printf("VA = %#06X - VPN = %#04X - OFF = %#04X - PFN = %03d - PA = %#06X\n", v_addr, (v_addr >> 8) & 0xFF, v_addr & 0xFF, (p_addr >> 8) & 0xFF, p_addr);
}