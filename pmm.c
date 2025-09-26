#include "pmm.h"
#include "vga.h"

#define PAGE_SIZE 4096
#define MAX_ORDER 10 // Max block size: 4KB * 2^10 = 4MB
#define KERNEL_RESERVED_END 0x200000 // Reserve the first 2MB for kernel/modules/metadata

typedef struct buddy_node {
    struct buddy_node* next;
} buddy_node_t;

static buddy_node_t* free_lists[MAX_ORDER + 1];
static inline uint32_t page_index_from_addr(void* addr) { return (uint32_t)addr / PAGE_SIZE; }
static inline void* addr_from_page_index(uint32_t index) { return (void*)(index * PAGE_SIZE); }
static inline uint32_t get_buddy_index(uint32_t index, uint32_t order) { return index ^ (1 << order); }

void pmm_init(uint32_t memory_end) {
    uint32_t total_pages = memory_end / PAGE_SIZE;

    // Initialize all free lists to be empty
    for (int i = 0; i <= MAX_ORDER; i++) {
        free_lists[i] = NULL;
    }

    // Correctly initialize by creating maximal aligned blocks
    uint32_t start_page = KERNEL_RESERVED_END / PAGE_SIZE;
    uint32_t current_page = start_page;

    while (current_page < total_pages) {
        // Find the largest possible block that fits and is aligned
        int order = MAX_ORDER;
        while (order >= 0) {
            uint32_t block_size = 1 << order;
            if ((current_page % block_size == 0) && (current_page + block_size <= total_pages)) {
                break;
            }
            order--;
        }

        // Add this maximal block to the correct free list
        buddy_node_t* block = (buddy_node_t*)addr_from_page_index(current_page);
        block->next = free_lists[order];
        free_lists[order] = block;

        current_page += (1 << order);
    }
}

void* pmm_alloc_page(void) {
    for (int order = 0; order <= MAX_ORDER; order++) {
        if (free_lists[order] != NULL) {
            // Found a block, take it from the list
            buddy_node_t* block = free_lists[order];
            free_lists[order] = block->next;

            // Split the block if it's larger than a single page (order 0)
            int current_order = order;
            while (current_order > 0) {
                current_order--;
                // The buddy is the second half of the split block
                buddy_node_t* buddy = (buddy_node_t*)((uint32_t)block + (PAGE_SIZE * (1 << current_order)));
                
                // Add the new buddy to the lower-order free list
                buddy->next = free_lists[current_order];
                free_lists[current_order] = buddy;
            }
            return block;
        }
    }
    return NULL; // Out of memory
}

void pmm_free_page(void* p) {
    uint32_t page_index = page_index_from_addr(p);
    int current_order = 0;

    // Coalesce with buddies as we move up the orders
    for (current_order = 0; current_order < MAX_ORDER; current_order++) {
        uint32_t buddy_index = get_buddy_index(page_index, current_order);
        buddy_node_t* buddy = (buddy_node_t*)addr_from_page_index(buddy_index);

        // Search the free list for the buddy
        buddy_node_t** list_head = &free_lists[current_order];
        int found_buddy = 0;
        while (*list_head) {
            if (*list_head == buddy) {
                *list_head = (*list_head)->next; // Remove buddy from the list
                found_buddy = 1;
                break;
            }
            list_head = &(*list_head)->next;
        }

        if (found_buddy) {
            // Buddy found, merge them. The new block's index is the lower of the two.
            page_index = (page_index < buddy_index) ? page_index : buddy_index;
        } else {
            // Buddy is not free, so stop merging
            break;
        }
    }

    // Add the final, possibly merged, block to the correct free list
    buddy_node_t* final_block = (buddy_node_t*)addr_from_page_index(page_index);
    final_block->next = free_lists[current_order];
    free_lists[current_order] = final_block;
}