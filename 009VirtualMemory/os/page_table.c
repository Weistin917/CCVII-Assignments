#include "page_table.h"
#include <stdlib.h>

// page table entry 
typedef struct {
    uint8_t pfn;
    uint8_t valid;
    // other flags
} pt_entry;

// page table 
struct pt {
    pt_entry *entries;
    uint8_t num_vpages;
};

/**
 * Creation of page table.
 * @param num_vpages: max capacity of the table
 * @return pointer to the created page table
 */
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

/**
 * Destruction of the page table
 * @param table: pointer to the page table
 */
void pt_destroy(pt* table) {
    free(table->entries);
    free(table);
}

/**
 * Get physical frame number.
 * @param table: pointer to the page table
 * @param vpn: virtual page number
 * @return physical page number if valid, otherwise -1
 */
int8_t pt_get_pfn(pt* table, uint8_t vpn) {
    if (vpn > table->num_vpages) return -1;
    return table->entries[vpn].pfn;
}

/**
 * Set page table entry as valid and assigns physical frame
 * @param table: page table pointer
 * @param vpn: virtual page number
 * @param pfn: assigned physical frame number
 */
void pt_set_entry(pt* table, uint8_t vpn, uint8_t pfn) {
    table->entries[vpn].pfn = pfn;
    table->entries[vpn].valid = 1;
}