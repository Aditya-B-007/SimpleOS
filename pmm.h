#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

//Initialize the physical memory manager.
void pmm_init(uint32_t memory_size, void* bitmap_addr);
// Allocate a block of a given order (2^order pages).
void* pmm_alloc_blocks(uint32_t order);
// Free a block of a given order.
void pmm_free_blocks(void* addr, uint32_t order);
// Convenience functions to allocate single pages.
void* pmm_alloc_page(void);
void pmm_free_page(void* addr);
// Get free memory information
uint32_t pmm_get_free_memory(void);
#endif