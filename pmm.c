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
    // Validate minimum memory requirement
    if (total_memory < KERNEL_START_ADDRESS + (2 * 1024 * 1024)) {
        // Insufficient memory - system cannot function
        return;
    }

    // Ensure memory alignment to 4KB boundaries
    memory_top = (KERNEL_START_ADDRESS + 4095) & ~4095;
    memory_limit = total_memory & ~4095;
    allocation_count = 0;

    // Initialize allocation table
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        allocations[i].address = 0;
        allocations[i].size = 0;
        allocations[i].used = 0;
    }
}

void* pmm_alloc(size_t size) {
    // Validate input parameters
    if (size == 0 || size < MIN_ALLOCATION_SIZE || size > MAX_ALLOCATION_SIZE) {
        return NULL;
    }

    // Align size to 16-byte boundary for better performance
    size = (size + 15) & ~15;

    // Check if we have space in allocation table
    if (allocation_count >= MAX_ALLOCATIONS) {
        return NULL;
    }

    // Ensure we don't overflow memory limits with safety margin
    if (memory_top >= memory_limit ||
        (memory_limit - memory_top) < size ||
        (memory_limit - memory_top) < (4 * 1024)) { // Keep 4KB safety margin
        return NULL;
    }

    // Validate memory_top alignment
    if (memory_top & 15) {
        memory_top = (memory_top + 15) & ~15;
    }

    // Find free allocation slot
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!allocations[i].used) {
            allocations[i].address = memory_top;
            allocations[i].size = size;
            allocations[i].used = 1;
            allocation_count++;

            void* ptr = (void*)memory_top;
            memory_top += size;

            // Clear allocated memory for security
            uint8_t* clear_ptr = (uint8_t*)ptr;
            for (size_t j = 0; j < size; j++) {
                clear_ptr[j] = 0;
            }

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

    // Validate address is within our memory range
    if (addr < KERNEL_START_ADDRESS || addr >= memory_limit) {
        return; // Invalid address
    }

    // Find and free the allocation
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (allocations[i].used && allocations[i].address == addr) {
            // Clear memory before freeing for security
            uint8_t* clear_ptr = (uint8_t*)ptr;
            for (size_t j = 0; j < allocations[i].size; j++) {
                clear_ptr[j] = 0;
            }

            // Mark as free
            allocations[i].address = 0;
            allocations[i].size = 0;
            allocations[i].used = 0;
            allocation_count--;
            return;
        }
    }
}

uint32_t pmm_get_free_memory(void) {
    if (memory_limit > memory_top) {
        return memory_limit - memory_top;
    }
    return 0;
}