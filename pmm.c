#include "pmm.h"
#include <stdint.h>

static uint32_t memory_top;
static uint32_t memory_limit;
static uint32_t allocation_count = 0;
#define KERNEL_START_ADDRESS 0x200000
#define MAX_ALLOCATIONS 1024
#define MIN_ALLOCATION_SIZE 16
#define MAX_ALLOCATION_SIZE (1024 * 1024)

typedef struct {
    uint32_t address;
    uint32_t size;
    int used;
} allocation_entry_t;

static allocation_entry_t allocations[MAX_ALLOCATIONS];

void pmm_init(uint32_t total_memory) {
    if (total_memory < KERNEL_START_ADDRESS + (1024 * 1024)) {
        return;
    }
    
    memory_top = KERNEL_START_ADDRESS;
    memory_limit = total_memory;
    allocation_count = 0;
    
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        allocations[i].used = 0;
    }
}

void* pmm_alloc(size_t size) {
    if (size == 0 || size < MIN_ALLOCATION_SIZE || size > MAX_ALLOCATION_SIZE) {
        return NULL;
    }
    
    size = (size + 15) & ~15;
    
    if (allocation_count >= MAX_ALLOCATIONS) {
        return NULL;
    }
    
    if (memory_top > memory_limit || (memory_limit - memory_top) < size) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!allocations[i].used) {
            allocations[i].address = memory_top;
            allocations[i].size = size;
            allocations[i].used = 1;
            allocation_count++;
            
            void* ptr = (void*)memory_top;
            memory_top += size;
            return ptr;
        }
    }
    
    return NULL;
}

void pmm_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    uint32_t addr = (uint32_t)ptr;
    
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (allocations[i].used && allocations[i].address == addr) {
            allocations[i].used = 0;
            allocation_count--;
            break;
        }
    }
}

uint32_t pmm_get_free_memory(void) {
    if (memory_limit > memory_top) {
        return memory_limit - memory_top;
    }
    return 0;
}