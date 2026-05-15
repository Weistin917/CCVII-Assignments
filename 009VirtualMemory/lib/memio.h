#ifndef MEMIO_H_
#define MEMIO_H_

void RAM_init(uint8_t num_vpage);
void RAM_free(void);
void memory_printout(void);
void addr_translate(uint16_t v_addr);

#endif  /*MEMIO_H_*/