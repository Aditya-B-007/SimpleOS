/* #ifndef PAGING_H
#define PAGING_H
#include <stdint.h>
#include "idt.h" 
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
#endif */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "idt.h"

// Page Table Entry
typedef struct page {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read-only if clear, read-write if set
    uint32_t user       : 1;   // Supervisor level only if clear
    uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
    uint32_t dirty      : 1;   // Has the page been written to since last refresh?
    uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
    uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];  // Array of pointers to pagetables.
    uint32_t physical_tables[1024]; // Array of physical addresses of pagetables.
    uint32_t physicalAddr; // The physical address of physical_tables
} page_directory_t;

void paging_install(void);
void paging_map(uint32_t phys, uint32_t virt, uint32_t flags);
void switch_page_directory(page_directory_t *dir);
extern page_directory_t* page_directory;

#endif
