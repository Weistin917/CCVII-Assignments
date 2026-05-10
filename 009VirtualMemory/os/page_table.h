#ifndef PAGE_TABLE_H_
#define PAGE_TABLE_H_

typedef struct pt pt;

pt* pt_create(unsigned char num_vpages);

#endif /*PAGE_TABLE_H_*/