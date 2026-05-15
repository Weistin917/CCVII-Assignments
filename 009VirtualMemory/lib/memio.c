/**
 * Memory input/output display
 * * RAM_init()
 * * RAM_map()
 * * summary()
 * * addr_translate()
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memio.h"
#include "mmu.h"
#include "random.h"

#define     CLR_RST     "\033[0m"   // color reset
static const char* STS_CLRS[] = {
    "\033[92m",     // FREE -> green
    "\033[91m",     // PREOCCUPIED -> red
    "\033[96m"      // OCCUPIED -> cyan
};

void RAM_map(void);
void frame_display(uint8_t frame_num, frame block);
void summary(void);

frame* current_ram_view;
uint8_t num_frames;

void RAM_init(uint8_t num_vpage) {
    num_frames = get_num_frames();
    current_ram_view = malloc(sizeof(frame) * num_frames);
    
    uint8_t occ_count;
    while (1) {
        occ_count = (uint8_t)rand_range(num_frames / 10, num_frames * 3 / 5);
        if ((num_vpage > 10) && (num_frames - occ_count >= num_vpage)) break;
        if (num_frames - occ_count >= 10) break;
    }

    ram_init(num_vpage, occ_count);

    assert((num_frames - occupied_count() >= num_vpage) || (num_frames - occ_count >= 10));

    printf("Physical RAM (%d) after random init:\n", num_frames);
    memory_printout();
    
    load_process(num_vpage);

    printf("Physical RAM (%d) after loading processes:\n", num_frames);
    memory_printout();
}

void RAM_free(void) {
    free(current_ram_view);
    ram_free();
}

void memory_printout(void) {
    get_RAM_view(current_ram_view);

    summary();
    RAM_map();
}

void RAM_map(void) {
    // print memory map with colors
    char line[101];
    memset(line, '-', 100);
    line[100] = '\0';
    printf("/%s\\\n", line);
    for (uint8_t i = 0; i < num_frames; i++) {
        if (!(i % 10)) {
            printf("|");
            frame_display(i, current_ram_view[i]);
        } else if (i % 10 == 9) {
            frame_display(i, current_ram_view[i]);
            printf("|\n");
        } else {
            frame_display(i, current_ram_view[i]);
        }
    }
    printf("\\%s/\n", line);
}

void frame_display(uint8_t frame_num, frame block) {
    char content[5];
    switch (block.state) {
        case 1:
            sprintf(content, "----");
            break;
        case 2:
            sprintf(content, "%#04X", block.v_page);
            break;
        default:
            memset(content, ' ', 4); 
            content[4] = '\0';
    }
    printf("%s|%03d:%s|%s", STS_CLRS[block.state], frame_num, content, CLR_RST);
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
    printf("VA = %#06X - %sVPN = %#04X%s - OFF = %#04X - %sPFN = %03d%s - PA = %#06X\n", v_addr, STS_CLRS[2], (v_addr >> 8) & 0xFF, CLR_RST, v_addr & 0xFF, STS_CLRS[0], (p_addr >> 8) & 0xFF, CLR_RST, p_addr);
}