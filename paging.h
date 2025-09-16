#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include "idt.h" //This is for interrupt handling
//Below is for defining page table entry
typedef struct pt_entry{
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t us:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t unused:7;// Amalgamation of unused and reserved bits
    uint32_t frame:20; //Frame address
} pt_entry_t;
void paging_install(void);
#endif
