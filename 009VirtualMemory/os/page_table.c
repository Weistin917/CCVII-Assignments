/**
 * Page table structure
 * * Page table struct
 * * Page table entry struct
 */
#include "page_table.h"
#include <stdlib.h>

typedef struct {
    unsigned char vpn;
    unsigned char pfn;
    unsigned char valid;
    // other flags
} pt_entry;

struct pt {
    pt_entry *entries;
    unsigned char num_vpages;
};

pt* pt_create(unsigned char num_vpages) {
    pt* table = malloc(sizeof(pt));
    if (table == NULL) return NULL;

    table->num_vpages = num_vpages;
    table->entries = calloc(table->num_vpages, sizeof(pt_entry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    for (unsigned char i = 0; i < num_vpages; i++) {
        table->entries->vpn = i;
    }
    return table;
}