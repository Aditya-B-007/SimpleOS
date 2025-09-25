#include "pmm.h"
#include "vga.h"
#include "paging.h"
#define MAX_ORDER 10 // Max block size will be 4KB * 2^10 = 4MB
#define PAGE_SIZE 4096
typedef struct buddy_node {
    struct buddy_node *next;
} buddy_node_t;

// An array of linked lists, one for each order.
static buddy_node_t* free_lists[MAX_ORDER + 1];
static uint8_t* memory_bitmap;
static uint32_t total_pages;

// Helper to get the buddy of a given block index.
static inline uint32_t get_buddy_index(uint32_t index, uint32_t order) {
    return index ^ (1 << order);
}

void pmm_init(uint32_t memory_size, void* bitmap_addr) {
    total_pages = memory_size / PAGE_SIZE;
    memory_bitmap = (uint8_t*)bitmap_addr;

    // Initialize all free lists to be empty.
    for (int i = 0; i <= MAX_ORDER; i++) {
        free_lists[i] = NULL;
    }

    // Reserve paging structures from management
    uint32_t paging_start = paging_get_reserved_start();
    uint32_t paging_end = paging_get_reserved_end();
    uint32_t paging_start_page = paging_start / PAGE_SIZE;
    uint32_t paging_end_page = (paging_end + PAGE_SIZE - 1) / PAGE_SIZE;

    vga_print_string("Reserving paging structures: 0x");
    vga_print_hex(paging_start);
    vga_print_string(" - 0x");
    vga_print_hex(paging_end);
    vga_print_string("\n");

    // Add all available memory into the allocator, skipping reserved regions
    uint32_t start_page = 0x100000 / PAGE_SIZE;
    for (uint32_t page = start_page; page < total_pages; ) {
        // Skip paging structures
        if (page >= paging_start_page && page < paging_end_page) {
            page = paging_end_page;
            continue;
        }
        // Find the largest order block we can create at this address.
        uint32_t order = MAX_ORDER;
        while (order > 0) {
            if ((page % (1 << order) == 0) && (page + (1 << order) <= total_pages)) {
                break;
            }
            order--;
        }
        
        // Add the block to the appropriate free list.
        buddy_node_t* block = (buddy_node_t*)(page * PAGE_SIZE);
        block->next = free_lists[order];
        free_lists[order] = block;
        page += (1 << order);
    }
}

void* pmm_alloc_blocks(uint32_t order) {
    if (order > MAX_ORDER) {
        return NULL;
    }

    // Find a free block, splitting larger ones if necessary.
    uint32_t current_order = order;
    while (current_order <= MAX_ORDER) {
        if (free_lists[current_order] != NULL) {
            // Found a block, break it down if it's larger than requested.
            buddy_node_t* block = free_lists[current_order];
            free_lists[current_order] = block->next;

            while (current_order > order) {
                current_order--;
                uint32_t buddy_addr = (uint32_t)block + (PAGE_SIZE * (1 << current_order));
                buddy_node_t* buddy = (buddy_node_t*)buddy_addr;
                
                // Add the buddy to its free list.
                buddy->next = free_lists[current_order];
                free_lists[current_order] = buddy;
            }
            return block;
        }
        current_order++;
    }

    return NULL; // No suitable block found
}

void pmm_free_blocks(void* addr, uint32_t order) {
    uint32_t page_index = (uint32_t)addr / PAGE_SIZE;

    // Merge the block with its buddy if the buddy is also free.
    while (order < MAX_ORDER) {
        uint32_t buddy_index = get_buddy_index(page_index, order);
        buddy_node_t* buddy = (buddy_node_t*)(buddy_index * PAGE_SIZE);
        
        // Search the free list for the buddy.
        buddy_node_t** list = &free_lists[order];
        int found_buddy = 0;
        while (*list) {
            if (*list == buddy) {
                *list = (*list)->next; // Remove buddy from the list
                found_buddy = 1;
                break;
            }
            list = &(*list)->next;
        }

        if (found_buddy) {
            // Merge and continue to the next order.
            page_index = (page_index < buddy_index) ? page_index : buddy_index;
            order++;
        } else {
            // Buddy is not free, stop merging.
            break;
        }
    }

    // Add the final (potentially merged) block to the free list.
    buddy_node_t* block = (buddy_node_t*)(page_index * PAGE_SIZE);
    block->next = free_lists[order];
    free_lists[order] = block;
}

// Convenience wrappers for single-page allocations.
void* pmm_alloc_page(void) {
    return pmm_alloc_blocks(0);
}

void pmm_free_page(void* addr) {
    pmm_free_blocks(addr, 0);
}

// Calculate total free memory available
uint32_t pmm_get_free_memory(void) {
    uint32_t total_free = 0;

    for (int order = 0; order <= MAX_ORDER; order++) {
        uint32_t block_size = PAGE_SIZE * (1 << order);
        uint32_t block_count = 0;

        buddy_node_t* current = free_lists[order];
        while (current != NULL) {
            block_count++;
            current = current->next;
        }

        total_free += block_count * block_size;
    }

    return total_free;
}