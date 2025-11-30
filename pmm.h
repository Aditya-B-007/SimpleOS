#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

typedef enum { PAGE_STATE_FREE=0, PAGE_STATE_USED=1, PAGE_STATE_RESERVED=2 } page_state_t;

typedef struct page_frame {
    struct page_frame* next;
    struct page_frame* prev;
    uint8_t order;
    page_state_t state;
    // ... other metadata

} page_frame_t;
void pmm_init(uint32_t memory_end);
void* pmm_alloc_page(void);
void pmm_free_page(void* p);
uint32_t pmm_get_free_memory(void);
#endif