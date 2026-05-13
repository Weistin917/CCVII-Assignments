/**
 * Page table structure
 * * Page table struct
 * * Page table entry struct
 */
#include "page_table.h"
#include <stdlib.h>

typedef struct {
    uint8_t pfn;
    uint8_t valid;
    // other flags
} pt_entry;

struct pt {
    pt_entry *entries;
    uint8_t num_vpages;
};

pt* pt_create(uint8_t num_vpages) {
    pt* table = malloc(sizeof(pt));
    if (table == NULL) return NULL;

    table->num_vpages = num_vpages;
    table->entries = calloc(table->num_vpages, sizeof(pt_entry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

void pt_destroy(pt* table) {
    free(table->entries);
    free(table);
}

int8_t pt_get_pfn(pt* table, uint8_t vpn) {
    if (vpn > table->num_vpages) return -1;
    return table->entries[vpn].pfn;
}

void pt_set_entry(pt* table, uint8_t vpn, uint8_t pfn) {
    table->entries[vpn].pfn = pfn;
    table->entries[vpn].valid = 1;
}