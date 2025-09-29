#ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include "idt.h" //This is for interrupt handling

// Paging memory layout constants
#define PAGING_STRUCTURES_START ((uint32_t)&page_directory)
#define PAGING_STRUCTURES_SIZE  (8192)  // 8KB for directory + table
extern pt_entry_t page_directory[1024];
extern pt_entry_t fpage_table[1024];
//Below is for defining page table entry
typedef struct pt_entry{
    uint32_t present:1;
    uint32_t rw:1;
    uint32_t user_supervisor:1;
    uint32_t pwt:1;               // Page Write-Through
    uint32_t pcd:1;               // Page Cache Disable
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t reserved:5;          // Reserved bits
    uint32_t frame:20;            // Frame address
} pt_entry_t;

// Function declarations
void paging_install(void);
uint32_t paging_get_reserved_start(void);
uint32_t paging_get_reserved_end(void);
#endif
