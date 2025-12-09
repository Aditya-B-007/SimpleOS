#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
void heap_init(uint32_t start_address, uint32_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);
#define malloc(size) kmalloc(size)
#define free(ptr) kfree(ptr)
#endif // HEAP_H