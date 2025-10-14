#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define MAX_ORDER 10 // 4KB * 2^10 = 4MB
#define KERNEL_RESERVED_END 0x200000U // bytes (2MB)

typedef enum { PAGE_STATE_FREE=0, PAGE_STATE_USED=1, PAGE_STATE_RESERVED=2 } page_state_t;

typedef struct page_frame {
    struct page_frame* next;
    struct page_frame* prev;
    uint8_t order;
    page_state_t state;
    // ... other metadata
} page_frame_t;

static page_frame_t* page_frame_database = NULL;
static page_frame_t* free_lists[MAX_ORDER + 1];
static uint32_t total_pages = 0;

static inline uint32_t idx_from_addr(void* addr) {
    return (uint32_t)((uintptr_t)addr / PAGE_SIZE);
}
static inline void* addr_from_idx(uint32_t idx) {
    return (void*)((uintptr_t)idx * PAGE_SIZE);
}
static inline uint32_t get_buddy_index(uint32_t index, uint32_t order) {
    return index ^ (1u << order);
}

void pmm_init(uint32_t memory_end_bytes) {
    total_pages = memory_end_bytes / PAGE_SIZE;
    size_t db_size = total_pages * sizeof(page_frame_t);
    uint32_t db_pages = (db_size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint32_t reserved_pages = (KERNEL_RESERVED_END / PAGE_SIZE) + db_pages;
    if (reserved_pages >= total_pages) {
        return;
    }
    page_frame_database = (page_frame_t*)(uintptr_t)KERNEL_RESERVED_END;
    uint32_t usable_start = reserved_pages;
    uint32_t usable_pages = total_pages - reserved_pages;
    for (uint32_t i = 0; i < total_pages; ++i) {
        page_frame_database[i].state = PAGE_STATE_FREE;
        page_frame_database[i].order = 0;
        page_frame_database[i].next = NULL;
        page_frame_database[i].prev = NULL;
    }
    for (uint32_t i = 0; i < reserved_pages; ++i) {
        page_frame_database[i].state = PAGE_STATE_RESERVED;
    }
    for (int i = 0; i <= MAX_ORDER; ++i) free_lists[i] = NULL;
    uint32_t current = usable_start;
    while (current < total_pages) {
        int order = MAX_ORDER;
        while (order >= 0) {
            uint32_t block_size = 1u << order;
            if ((current % block_size) == 0 && (current + block_size) <= total_pages) break;
            --order;
        }
        if (order < 0) { // fallback to single page
            order = 0;
        }
        page_frame_t* f = &page_frame_database[current];
        f->order = (uint8_t)order;
        f->state = PAGE_STATE_FREE;
        // push to free list head
        f->prev = NULL;
        f->next = free_lists[order];
        if (free_lists[order]) free_lists[order]->prev = f;
        free_lists[order] = f;

        current += (1u << order);
    }
}

void* pmm_alloc_page(void) {
    for (int order = 0; order <= MAX_ORDER; ++order) {
        if (!free_lists[order]) continue;
        page_frame_t* block = free_lists[order];
        free_lists[order] = block->next;
        if (free_lists[order]) free_lists[order]->prev = NULL;
        int cur_order = order;
        uint32_t base_idx = (uint32_t)(block - page_frame_database);
        while (cur_order > 0) {
            cur_order--;
            uint32_t buddy_idx = base_idx + (1u << cur_order);
            page_frame_t* buddy = &page_frame_database[buddy_idx];
            buddy->order = (uint8_t)cur_order;
            buddy->state = PAGE_STATE_FREE;
            buddy->next = free_lists[cur_order];
            if (free_lists[cur_order]) free_lists[cur_order]->prev = buddy;
            buddy->prev = NULL;
            free_lists[cur_order] = buddy;
        }

        block->order = 0;
        block->state = PAGE_STATE_USED;
        block->next = block->prev = NULL;
        uint32_t idx = (uint32_t)(block - page_frame_database);
        return addr_from_idx(idx);
    }
    return NULL;
}

void pmm_free_page(void* p) {
    if (!p) return;
    uintptr_t addr = (uintptr_t)p;
    if (addr % PAGE_SIZE) return; 

    uint32_t idx = (uint32_t)(addr / PAGE_SIZE);
    if (idx >= total_pages) return;

    page_frame_t* frame = &page_frame_database[idx];
    if (frame->state == PAGE_STATE_RESERVED) return;

    frame->state = PAGE_STATE_FREE;
    frame->order = 0;

    for (int order = 0; order < MAX_ORDER; ++order) {
        uint32_t buddy_idx = get_buddy_index(idx, order);
        if (buddy_idx >= total_pages) break;
        page_frame_t* buddy = &page_frame_database[buddy_idx];

        if (buddy->state != PAGE_STATE_FREE || buddy->order != order) {
            break; // can't merge
        }
        if (buddy->prev) buddy->prev->next = buddy->next;
        else free_lists[order] = buddy->next;
        if (buddy->next) buddy->next->prev = buddy->prev;
        if (buddy_idx < idx) idx = buddy_idx;
        frame = &page_frame_database[idx];
        frame->order = (uint8_t)(order + 1);
        frame->state = PAGE_STATE_FREE;
    }
    int final_order = frame->order;
    frame->next = free_lists[final_order];
    if (free_lists[final_order]) free_lists[final_order]->prev = frame;
    frame->prev = NULL;
    free_lists[final_order] = frame;
}

uint32_t pmm_get_free_memory(void) {
    uint32_t total_free = 0;
    for (int order = 0; order <= MAX_ORDER; ++order) {
        uint32_t block_pages = (1u << order);
        page_frame_t* cur = free_lists[order];
        while (cur) {
            total_free += block_pages * PAGE_SIZE;
            cur = cur->next;
        }
    }
    return total_free;
}
