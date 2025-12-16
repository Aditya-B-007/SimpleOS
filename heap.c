#include <heap.h>
#include <sync.h>
#define HEAP_ALIGNMENT 4
#define HEAP_MAGIC 0x12345678

typedef struct heap_segment_header{
    uint32_t magic; // Should always be HEAP_MAGIC
    size_t size;
    bool free;
    struct heap_segment_header* next;
    struct heap_segment_header* prev;
}heap_header_t;

static void* heap_start_address = NULL;
static size_t heap_size = 0;
static heap_header_t* first_segment = NULL;
static spinlock_t heap_lock;

static inline size_t align(size_t size) {
    return (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

void heap_init(uint32_t start_address, uint32_t size){
    if (size < sizeof(heap_header_t) + HEAP_ALIGNMENT) {
        // Not enough space for even one small allocation.
        return;
    }
    heap_start_address = (void*)start_address;
    heap_size = size;
    first_segment = (heap_header_t*)heap_start_address;
    spinlock_init(&heap_lock);
    first_segment->magic = HEAP_MAGIC;
    first_segment->size = size - sizeof(heap_header_t);
    first_segment->free = true;
    first_segment->next = NULL;
    first_segment->prev = NULL;
}

void* heap_alloc(size_t size){
    spinlock_acquire(&heap_lock);

    if (size == 0) {
        spinlock_release(&heap_lock);
        return NULL;
    }
    size_t aligned_size = align(size);
    heap_header_t* current = first_segment;
    while (current != NULL) {
        if (current->free && current->size >= aligned_size) {
            // Check if the remaining space is large enough for a new header and a minimal allocation.
            if (current->size >= aligned_size + sizeof(heap_header_t) + HEAP_ALIGNMENT) { // Smallest alloc is HEAP_ALIGNMENT
                heap_header_t* new_segment = (heap_header_t*)((uint8_t*)current + sizeof(heap_header_t) + aligned_size);
                new_segment->magic = HEAP_MAGIC;
                new_segment->size = current->size - aligned_size - sizeof(heap_header_t);
                new_segment->free = true;
                new_segment->next = current->next;
                new_segment->prev = current;

                if (current->next != NULL) {
                    current->next->prev = new_segment;
                }
                current->next = new_segment;
                current->size = aligned_size;
            }
            current->free = false;
            spinlock_release(&heap_lock);
            return (void*)((uint8_t*)current + sizeof(heap_header_t));
        }
        current = current->next;
    }
    // Out of memory
    spinlock_release(&heap_lock);
    return NULL;
}

void heap_free(void* ptr){
    spinlock_acquire(&heap_lock);

    if (ptr == NULL) {
        spinlock_release(&heap_lock);
        return;
    }
    heap_header_t* header = (heap_header_t*)((uint8_t*)ptr - sizeof(heap_header_t));
    if (header->magic != HEAP_MAGIC) {
        // PANIC("Heap corruption detected!");
        spinlock_release(&heap_lock);
        return; // Or better, panic.
    }

    header->free = true;
    heap_header_t* next = header->next;
    if (next != NULL && next->free) {
        header->size += sizeof(heap_header_t) + next->size;
        header->next = next->next;
        if (next->next != NULL) {
            next->next->prev = header;
        }
    }

    // Coalesce with previous block if it's free
    heap_header_t* prev = header->prev;
    if (prev != NULL && prev->free) {
        prev->size += sizeof(heap_header_t) + header->size;
        prev->next = header->next;
        if (prev->next != NULL) { // This was header->next before, which is wrong
            prev->next->prev = prev;
        }
    }

    spinlock_release(&heap_lock);
}

void* kmalloc(size_t size) {
    return heap_alloc(size);
}

void kfree(void* ptr) {
    heap_free(ptr);
}
