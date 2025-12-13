#include <stddef.h>
#include <stdint.h>
#include "pmm.h"

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (num--) *d++ = *s++;
    return dest;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// Simple heap implementation for malloc/free
#define HEAP_SIZE 1024 * 1024 // 1MB
static uint8_t heap_memory[HEAP_SIZE];
static size_t heap_offset = 0;

void* malloc(size_t size) {
    if (heap_offset + size > HEAP_SIZE) return NULL;
    void* ptr = &heap_memory[heap_offset];
    heap_offset += size;
    return ptr;
}

void free(void* ptr) {
    (void)ptr; // No-op for simple bump allocator
}

// Stub for syscalls
void syscalls_install(void) {}