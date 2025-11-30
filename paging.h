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
    uint32_t present:1;         // Bit 0: Present
    uint32_t rw:1;              // Bit 1: Read/Write
    uint32_t user_supervisor:1; // Bit 2: User/Supervisor
    uint32_t pwt:1;             // Bit 3: Page Write-Through
    uint32_t pcd:1;             // Bit 4: Page Cache Disable
    uint32_t accessed:1;        // Bit 5: Accessed
    uint32_t dirty:1;           // Bit 6: Dirty
    uint32_t pat:1;             // Bit 7: Page Attribute Table
    uint32_t global:1;          // Bit 8: Global
    uint32_t avail:3;           // Bits 9-11: Available for software use
    uint32_t frame:20;          // Bits 12-31: Frame address (shifted right 12 bits)
} pt_entry_t;

// Function declarations
void paging_install(void);
uint32_t paging_get_reserved_start(void);
uint32_t paging_get_reserved_end(void);
#endif
