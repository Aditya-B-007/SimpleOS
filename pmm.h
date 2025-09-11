#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

void pmm_init(uint32_t memory_size);
void* pmm_alloc(size_t size);
void pmm_free(void* ptr);
uint32_t pmm_get_free_memory(void);

#endif