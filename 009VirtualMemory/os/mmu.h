#ifndef MMU_H_
#define MMU_H_

#include <stdint.h>

typedef struct frame {
    uint8_t state;
    uint8_t v_page;
} frame;

uint8_t get_num_frames(void);
void ram_init(uint8_t num_vpage, uint8_t occ_count);
void ram_free(void);
void get_RAM_view(frame* buffer);
int8_t allocate_frame(void);
void load_process(uint8_t num_vpage);
int16_t translate(uint16_t v_addr);
uint8_t occupied_count(void);

#endif  /*MMU_H_*/