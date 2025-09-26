#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

void pmm_init(uint32_t memory_end);
void* pmm_alloc_page(void);
void pmm_free_page(void* p);

#endif