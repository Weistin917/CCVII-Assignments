#ifndef PAGE_TABLE_H_
#define PAGE_TABLE_H_

#include <stdint.h>

typedef struct pt pt;

pt* pt_create(uint8_t num_vpages);
void pt_destroy(pt* table);
int8_t pt_get_pfn(pt* table, uint8_t vpn);
void pt_set_entry(pt* table, uint8_t vpn, uint8_t pfn);

#endif /*PAGE_TABLE_H_*/