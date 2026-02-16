#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
extern void syscall_handler(registers_t *r);
extern void schedule(registers_t *r);
extern void window_handle_key(char key);
extern void shell_handle_input(char c);
extern void dirty_rect_add(int x, int y, int width, int height);

// ==========================================
// GLOBAL DEFINITIONS (Moved from kernel.c/others)
// ==========================================
struct Widget* widget_list_head = NULL;
struct Window* window_list_head = NULL;
struct Window* window_list_tail = NULL;
struct VbeInfoBlock {
    uint16_t attributes;
    uint8_t windowA, windowB;
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t segmentA, segmentB;
    uint32_t winFuncPtr;
    uint16_t pitch;
    uint16_t width, height;
    uint8_t wChar, yChar, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;
    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed));
struct VbeInfoBlock vbe_info_block = {0};
//struct VbeModeInfo vbe_mode_info = {0};
// ==========================================
// FILE: lib.c (Standard Library Helpers)
// ==========================================
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

// NOTE: Simple heap disabled in favor of heap.c
// #define HEAP_SIZE 1024 * 1024 
// static uint8_t heap_memory[HEAP_SIZE];
// static size_t heap_offset = 0;
// void* malloc(size_t size) { ... }
// void free(void* ptr) { ... }

// Stub removed in favor of syscall.c implementation
// void syscalls_install(void) {}

// ==========================================
// FILE: font.c
// ==========================================
const uint8_t font[256][16] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x00 Null
	{0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x01 SOH
	{0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x02 STX
	{0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x03 ETX
	{0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x04 EOT
	{0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x7C, 0x38, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x05 ENQ
	{0x10, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x06 ACK
	{0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x07 BEL
	{0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x08 BS
	{0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x09 HT
	{0x00, 0x3C, 0x7E, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0A LF
	{0x0C, 0x18, 0x30, 0x7E, 0x7E, 0x7E, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0B VT
	{0x38, 0x7C, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0C FF
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0D CR
	{0x0C, 0x1E, 0x3F, 0x3F, 0x1E, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0E SO
	{0x38, 0x7C, 0xFE, 0x38, 0x38, 0xFE, 0x7C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0F SI
	{0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x10 DLE
	{0x30, 0x78, 0x78, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x11 DC1
	{0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x12 DC2
	{0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x13 DC3
	{0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x14 DC4
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x15 NAK
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x16 SYN
	{0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x17 ETB
	{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x18 CAN
	{0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x19 EM
	{0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1A SUB
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1B ESC
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1C FS
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1D GS
	{0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1E RS
	{0x18, 0x18, 0x18, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x1F US
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x20 Space
	{0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x21 !
	{0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x22 "
	{0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x23 #
	{0x38, 0x7C, 0xA0, 0x78, 0x2C, 0xF8, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x24 $
	{0xC6, 0xC6, 0x0C, 0x18, 0x30, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x25 %
	{0x38, 0x7C, 0x80, 0x80, 0x7C, 0x86, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x26 &
	{0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x27 '
	{0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x28 (
	{0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x29 )
	{0x00, 0x6C, 0x38, 0xFE, 0x38, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2A *
	{0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2B +
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2C ,
	{0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2D -
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2E .
	{0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x2F /
	{0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x30 0
	{0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x31 1
	{0x7C, 0x82, 0x02, 0x7C, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x32 2
	{0x7C, 0x02, 0x02, 0x3C, 0x02, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x33 3
	{0x84, 0x84, 0x84, 0xFE, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x34 4
	{0xFE, 0x80, 0x80, 0xFC, 0x02, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x35 5
	{0x7C, 0x80, 0x80, 0xFC, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x36 6
	{0xFE, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x37 7
	{0x7C, 0x82, 0x82, 0x7C, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x38 8
	{0x7C, 0x82, 0x82, 0x7E, 0x02, 0x02, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x39 9
	{0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3A :
	{0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3B ;
	{0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3C <
	{0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3D =
	{0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3E >
	{0x7C, 0x82, 0x02, 0x0C, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x3F ?
	{0x7C, 0x82, 0xBA, 0xAA, 0xAA, 0xB8, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x40 @
	{0x7C, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x41 A
	{0xFC, 0x82, 0x82, 0xFC, 0x82, 0x82, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x42 B
	{0x7C, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x43 C
	{0xF8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x44 D
	{0xFE, 0x80, 0x80, 0xF8, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x45 E
	{0xFE, 0x80, 0x80, 0xF8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x46 F
	{0x7C, 0x82, 0x80, 0x8E, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x47 G
	{0x82, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x48 H
	{0x7C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x49 I
	{0x0E, 0x02, 0x02, 0x02, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4A J
	{0x82, 0x84, 0x88, 0xF0, 0x88, 0x84, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4B K
	{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4C L
	{0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4D M
	{0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4E N
	{0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x4F O
	{0xFC, 0x82, 0x82, 0xFC, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x50 P
	{0x7C, 0x82, 0x82, 0x82, 0x8A, 0x84, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x51 Q
	{0xFC, 0x82, 0x82, 0xFC, 0x88, 0x84, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x52 R
	{0x7C, 0x80, 0x80, 0x7C, 0x02, 0x02, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x53 S
	{0xFE, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x54 T
	{0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x55 U
	{0x82, 0x82, 0x82, 0x44, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x56 V
	{0x82, 0x82, 0x82, 0x92, 0x92, 0xAA, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x57 W
	{0x82, 0x82, 0x44, 0x38, 0x44, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x58 X
	{0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x59 Y
	{0xFE, 0x04, 0x08, 0x10, 0x20, 0x40, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5A Z
	{0x7C, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5B [
	{0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5C '\'
	{0x7C, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5D ]
	{0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5E ^
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x5F _
	{0x30, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x60 `
	{0x00, 0x00, 0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x61 a
	{0x80, 0x80, 0xF8, 0x84, 0x84, 0x84, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x62 b
	{0x00, 0x00, 0x78, 0x80, 0x80, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x63 c
	{0x04, 0x04, 0x7C, 0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x64 d
	{0x00, 0x00, 0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x65 e
	{0x0C, 0x10, 0x10, 0xF8, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x66 f
	{0x00, 0x00, 0x7C, 0x84, 0x84, 0x7C, 0x04, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x67 g
	{0x80, 0x80, 0xF8, 0x84, 0x84, 0x84, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x68 h
	{0x30, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x69 i
	{0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0x8C, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6A j
	{0x80, 0x80, 0x88, 0x90, 0xA0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6B k
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6C l
	{0x00, 0x00, 0xCC, 0xAA, 0xAA, 0xAA, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6D m
	{0x00, 0x00, 0xF8, 0x84, 0x84, 0x84, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6E n
	{0x00, 0x00, 0x78, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x6F o
	{0x00, 0x00, 0xF8, 0x84, 0x84, 0xF8, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x70 p
	{0x00, 0x00, 0x7C, 0x84, 0x84, 0x7C, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x71 q
	{0x00, 0x00, 0xAC, 0x94, 0x90, 0x90, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x72 r
	{0x00, 0x00, 0x70, 0x88, 0x70, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x73 s
	{0x10, 0x10, 0xF8, 0x10, 0x10, 0x10, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x74 t
	{0x00, 0x00, 0x84, 0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x75 u
	{0x00, 0x00, 0x84, 0x84, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x76 v
	{0x00, 0x00, 0x84, 0x84, 0x94, 0x94, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x77 w
	{0x00, 0x00, 0x84, 0x50, 0x28, 0x50, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x78 x
	{0x00, 0x00, 0x84, 0x84, 0x84, 0x7C, 0x04, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x79 y
	{0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7A z
	{0x1C, 0x30, 0x30, 0x60, 0x30, 0x30, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7B {
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7C |
	{0xE0, 0x30, 0x30, 0x18, 0x30, 0x30, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7D }
	{0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7E ~
	{0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x7F DEL
	{0x7C, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x80 Ç
	{0x84, 0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x81 ü
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x82 é
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x83 â
	{0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x84 ä
	{0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x85 à
	{0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x86 å
	{0x78, 0x80, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x87 ç
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x88 ê
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x89 ë
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8A è
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8B ï
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8C î
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8D ì
	{0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8E Ä
	{0x78, 0x84, 0x84, 0x78, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8F Å
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x90 É
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x91 æ
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x92 Æ
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x93 ô
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x94 ö
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x95 ò
	{0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x96 û
	{0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x97 ù
	{0x84, 0x84, 0x84, 0x7C, 0x04, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x98 ÿ
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x99 Ö
	{0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9A Ü
	{0x7C, 0x80, 0xFC, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9B ø
	{0xFC, 0x80, 0x80, 0xF8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9C £
	{0x7C, 0x80, 0xFC, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9D Ø
	{0x82, 0x44, 0x38, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9E ×
	{0x80, 0x80, 0xF8, 0x84, 0x84, 0x84, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9F ƒ
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA0 á
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA1 í
	{0x78, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA2 ó
	{0x84, 0x84, 0x84, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA3 ú
	{0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA4 ñ
	{0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA5 Ñ
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA6 ª
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA7 º
	{0x7C, 0x82, 0x02, 0x0C, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA8 ¿
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA9 ®
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAA ¬
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAB ½
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAC ¼
	{0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAD ¡
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAE «
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xAF »
	{0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB0 ░
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB1 ▒
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB2 ▓
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB3 │
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB4 ┤
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB5 ╡
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB6 ╢
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB7 ╖
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB8 ╕
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB9 ╣
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBA ║
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBB ╗
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBC ╝
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBD ╜
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBE ╛
	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBF ┐
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC0 └
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC1 ┴
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC2 ┬
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC3 ├
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC4 ─
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC5 ┼
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC6 ╞
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC7 ╟
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC8 ╚
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xC9 ╔
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCA ╩
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCB ╦
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCC ╠
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCD ═
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCE ╬
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xCF ╧
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD0 ╨
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD1 ╤
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD2 ╥
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD3 ╙
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD4 ╘
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD5 ╒
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD6 ╓
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD7 ╫
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD8 ╪
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xD9 ┘
	{0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDA ┌
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDB █
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDC ▄
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDD ▌
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDE ▐
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xDF ▀
	{0x7C, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE0 α
	{0xFC, 0x82, 0x82, 0xFC, 0x82, 0x82, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE1 ß
	{0x7C, 0x82, 0x80, 0x8E, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE2 Γ
	{0x82, 0x82, 0x82, 0xFE, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE3 π
	{0xFE, 0x80, 0x80, 0xF8, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE4 Σ
	{0xFE, 0x80, 0x80, 0xF8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE5 σ
	{0x7C, 0x82, 0x82, 0x7C, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE6 µ
	{0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE7 τ
	{0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE8 Φ
	{0xFE, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xE9 Θ
	{0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xEA Ω
	{0x82, 0x82, 0x82, 0x44, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xEB δ
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xEC ∞
	{0x82, 0x82, 0x44, 0x38, 0x44, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xED φ
	{0x78, 0x84, 0xF4, 0x80, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xEE ε
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xEF ∩
	{0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF0 ≡
	{0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF1 ±
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF2 ≥
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF3 ≤
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF4 ⌠
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF5 ⌡
	{0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF6 ÷
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF7 ≈
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF8 °
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xF9 ∙
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFA ·
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFB √
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFC ⁿ
	{0x7C, 0x82, 0x02, 0x7C, 0x80, 0x80, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFD ²
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFE ■
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xFF (NBSP)
};

// ==========================================
// FILE: gdt.c
// ==========================================
#define GDT_ENTRIES 5
static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_install(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                    
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    

    gdt_flush();
}

void gdt_flush(void) {
    __asm__ __volatile__ (
        "lgdt %0\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "ljmp $0x08, $1f\n\t"
        "1:\n\t"
        :
        : "m"(gp)
        : "eax"
    );
}

// ==========================================
// FILE: idt.c
// ==========================================
static struct idt_entry idt[256];
static struct idt_ptr ip;
static isr_t interrupt_handlers[256];
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr128(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
// Exception messages
static const char *exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "Reserved"
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void isr_handler(registers_t *r) {
    if (r == NULL) return;

    if (r->int_no == 128) {
        syscall_handler(r);
        return;
    }

    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
        return; 
    }

    console_write("\n[ISR] Interrupt: ");
    console_write_dec(r->int_no);
    console_write("\n");

    if (r->int_no < 32) {
        if (r->int_no < sizeof(exception_messages) / sizeof(exception_messages[0])) {
            console_write("Exception: ");
            console_write(exception_messages[r->int_no]);
            console_write("\n");
        } else {
            console_write("Unknown Exception\n");
        }
        console_write("System Halted.\n");
        __asm__ __volatile__("cli");
        for(;;) { __asm__ __volatile__("hlt"); }
    }
}

static void (*irq_routines[16])(registers_t *r);

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_install_handler(int irq, void (*handler)(registers_t *r)) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = handler;
    }
}

void irq_handler(registers_t *r) {
    if (interrupt_handlers[r->int_no]) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
        return;
    }
    
    if (r->int_no >= 32 && r->int_no <= 47) {
        void (*handler)(registers_t *r) = irq_routines[r->int_no - 32];
        if (handler) {
            handler(r);
        }
    }

    if (r->int_no >= 40) outb(0xA0, 0x20); // Slave
    outb(0x20, 0x20); // Master
}

void idt_install(void) {
    ip.limit = sizeof(struct idt_entry) * 256 - 1;
    ip.base = (uint32_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);
    memset(interrupt_handlers, 0, sizeof(isr_t) * 256);
    memset(irq_routines, 0, sizeof(void *) * 16);

    for(int i=0; i<256; i++) idt_set_gate(i, 0, 0, 0);

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page Fault
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    // Remap PIC
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // MASK ALL INTERRUPTS INITIALLY
    outb(0x21, 0xFF); 
    outb(0xA1, 0xFF);

    // Install IRQs
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    // Syscall
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEE);

    __asm__ __volatile__("lidt %0" : : "m"(ip));
}

// ==========================================
// FILE: graphics.c
// ==========================================
void put_pixel(FrameBuffer* fb, int32_t x, int32_t y, uint32_t color) {
    if (x < 0 || x >= (int32_t)fb->width || y < 0 || y >= (int32_t)fb->height) return; 
    
    uint8_t* pixel = (uint8_t*)fb->address + y * fb->pitch + x * fb->bytesPerPixel;

    if (fb->bitsPerPixel == 24) {
        pixel[0] = color & 0xFF;         
        pixel[1] = (color >> 8) & 0xFF;  
        pixel[2] = (color >> 16) & 0xFF; 
    } else if (fb->bitsPerPixel == 32) {
        *(uint32_t*)pixel = color;
    }
}

void clear_screen(FrameBuffer* fb, uint32_t color) {
    for (uint32_t y = 0; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            put_pixel(fb, x, y, color);
        }
    }
}
void draw_circle(FrameBuffer* fb, int32_t xc, int32_t yc, int32_t r, uint32_t color) {
    int32_t x = r;
    int32_t y = 0;
    int32_t err = 3 - (2 * r);

    while (x >= y) {
        put_pixel(fb, xc + x, yc + y, color);
        put_pixel(fb, xc + y, yc + x, color);
        put_pixel(fb, xc - y, yc + x, color);
        put_pixel(fb, xc - x, yc + y, color);
        put_pixel(fb, xc - x, yc - y, color);
        put_pixel(fb, xc - y, yc - x, color);
        put_pixel(fb, xc + y, yc - x, color);
        put_pixel(fb, xc + x, yc - y, color);

        if (err > 0) {
            err += 4 * (y - x) + 10;
            x--;
        } else {
            err += 4 * y + 6;
        }
        y++;
    }
}
void draw_line(FrameBuffer* fb, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    int32_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int32_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx - dy;
    while (1) {
        put_pixel(fb, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    if (width <= 0 || height <= 0) return;
    int32_t x1 = x + width - 1;
    int32_t y1 = y + height - 1;
    draw_line(fb, x, y, x1, y, color);       
    draw_line(fb, x, y1, x1, y1, color);     
    draw_line(fb, x, y, x, y1, color);       
    draw_line(fb, x1, y, x1, y1, color);     
}

void fill_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    for (int32_t j = y; j < y + height; j++) {
        for (int32_t i = x; i < x + width; i++) {
            put_pixel(fb, i, j, color);
        }
    }
}

void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, int32_t x, int32_t y, uint32_t color) {
    if (!bmp || !bmp->data) return;
    for (uint32_t j = 0; j < bmp->height; j++) {
        for (uint32_t i = 0; i < bmp->width; i++) {
            uint8_t pixel = bmp->data[j * bmp->width + i];
            if (pixel) { 
                put_pixel(fb, x + i, y + j, color);
            }
        }
    }
}

void draw_char(FrameBuffer* fb, Font* font, char c, int32_t x, int32_t y, uint32_t color) {
    if (!font || !font->bitmap) return;
    const uint8_t* glyph = font->bitmap[(unsigned char)c];
    for (uint32_t cy = 0; cy < font->char_height; cy++) {
        uint8_t row = glyph[cy];
        for (uint32_t cx = 0; cx < font->char_width; cx++) {
            if ((row >> (7 - cx)) & 1) {
                put_pixel(fb, x + cx, y + cy, color);
            }
        }
    }
}

void draw_string(FrameBuffer* fb, Font* font, const char* str, int32_t x, int32_t y, uint32_t color) {
    while (*str) {
        draw_char(fb, font, *str, x, y, color);
        x += font->char_width;
        str++;
    }
}

// ==========================================
// FILE: vga.c
// ==========================================
static volatile uint16_t* const vga_buffer = (uint16_t*)0xB8000;
static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
}

void vga_setcolor(uint8_t color) {
    vga_color = color;
}
void vga_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

void vga_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y-1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
            vga_row = VGA_HEIGHT - 1;
        }
    } else {
        vga_putentryat(c, vga_color, vga_column, vga_row);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
                vga_row = VGA_HEIGHT - 1;
            }
        }
    }
}

void vga_print_string(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

void vga_print_hex(uint32_t n) {
    vga_print_string("0x");
    char hex_digits[] = "0123456789ABCDEF";
    char hex_string[9];
    hex_string[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        hex_string[i] = hex_digits[n & 0xF];
        n >>= 4;
    }
    vga_print_string(hex_string);
}

void vga_print_dec(uint32_t n) {
    if (n == 0) {
        vga_putchar('0');
        return;
    }
    char dec_string[11];
    int i = 0;
    while (n > 0) {
        dec_string[i++] = '0' + (n % 10);
        n /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(dec_string[j]);
    }
}

// ==========================================
// FILE: pmm.c
// ==========================================
#define MAX_ORDER 10
#define KERNEL_RESERVED_END 0x200000U
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
    if (reserved_pages >= total_pages) return;
    
    page_frame_database = (page_frame_t*)(uintptr_t)KERNEL_RESERVED_END;
    uint32_t usable_start = reserved_pages;
    
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
        if (order < 0) order = 0;
        
        page_frame_t* f = &page_frame_database[current];
        f->order = (uint8_t)order;
        f->state = PAGE_STATE_FREE;
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

void* pmm_alloc_pages(uint32_t count) {
    int order = 0;
    while ((1u << order) < count) order++;
    if (order > MAX_ORDER) return NULL;

    for (int i = order; i <= MAX_ORDER; ++i) {
        if (!free_lists[i]) continue;
        page_frame_t* block = free_lists[i];
        free_lists[i] = block->next;
        if (free_lists[i]) free_lists[i]->prev = NULL;

        while (i > order) {
            i--;
            uint32_t base_idx = (uint32_t)(block - page_frame_database);
            uint32_t buddy_idx = base_idx + (1u << i);
            page_frame_t* buddy = &page_frame_database[buddy_idx];
            buddy->order = (uint8_t)i;
            buddy->state = PAGE_STATE_FREE;
            buddy->next = free_lists[i];
            if (free_lists[i]) free_lists[i]->prev = buddy;
            buddy->prev = NULL;
            free_lists[i] = buddy;
        }
        block->order = (uint8_t)order;
        block->state = PAGE_STATE_USED;
        return addr_from_idx((uint32_t)(block - page_frame_database));
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

        if (buddy->state != PAGE_STATE_FREE || buddy->order != order) break;
        
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

// ==========================================
// FILE: heap.c
// ==========================================
#define HEAP_ALIGNMENT 4
#define HEAP_MAGIC 0x12345678

typedef struct heap_segment_header{
    uint32_t magic; 
    size_t size;
    bool free;
    struct heap_segment_header* next;
    struct heap_segment_header* prev;
} heap_header_t;

static void* heap_start_address = NULL;
static size_t heap_size = 0;
static heap_header_t* first_segment = NULL;
static spinlock_t heap_lock;

static inline size_t align(size_t size) {
    return (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

void heap_init(uint32_t start_address, uint32_t size){
    if (size < sizeof(heap_header_t) + HEAP_ALIGNMENT) return;
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
            if (current->size >= aligned_size + sizeof(heap_header_t) + HEAP_ALIGNMENT) { 
                heap_header_t* new_segment = (heap_header_t*)((uint8_t*)current + sizeof(heap_header_t) + aligned_size);
                new_segment->magic = HEAP_MAGIC;
                new_segment->size = current->size - aligned_size - sizeof(heap_header_t);
                new_segment->free = true;
                new_segment->next = current->next;
                new_segment->prev = current;

                if (current->next != NULL) current->next->prev = new_segment;
                current->next = new_segment;
                current->size = aligned_size;
            }
            current->free = false;
            spinlock_release(&heap_lock);
            return (void*)((uint8_t*)current + sizeof(heap_header_t));
        }
        current = current->next;
    }
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
        spinlock_release(&heap_lock);
        return; 
    }

    header->free = true;
    heap_header_t* next = header->next;
    if (next != NULL && next->free) {
        header->size += sizeof(heap_header_t) + next->size;
        header->next = next->next;
        if (next->next != NULL) next->next->prev = header;
    }

    heap_header_t* prev = header->prev;
    if (prev != NULL && prev->free) {
        prev->size += sizeof(heap_header_t) + header->size;
        prev->next = header->next;
        if (prev->next != NULL) prev->next->prev = prev;
    }
    spinlock_release(&heap_lock);
}

void* kmalloc(size_t size) { return heap_alloc(size); }
void kfree(void* ptr) { heap_free(ptr); }

// ==========================================
// FILE: paging.c
// ==========================================
page_directory_t* page_directory = 0;

void paging_map(uint32_t phys, uint32_t virt, uint32_t flags) {
    (void)flags;
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (!page_directory->tables[pd_index]) {
        page_table_t* new_table = (page_table_t*)pmm_alloc_page();
        memset(new_table, 0, sizeof(page_table_t));
        page_directory->tables[pd_index] = new_table;
        page_directory->physical_tables[pd_index] = ((uint32_t)new_table) | 0x7;
    }

    page_table_t* table = page_directory->tables[pd_index];
    table->pages[pt_index].frame = phys >> 12;
    table->pages[pt_index].present = 1;
    table->pages[pt_index].rw = 1;
    table->pages[pt_index].user = 1;
}

void switch_page_directory(page_directory_t *dir) {
    uint32_t cr0;
    asm volatile("mov %0, %%cr3" :: "r"(&dir->physical_tables));
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void page_fault_handler(registers_t *r) {
    (void)r;
    uint32_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
    
    console_write("\n[CRITICAL] PAGE FAULT at 0x");
    console_write_dec(faulting_address);
    console_write("\nSystem Halted.\n");
    __asm__ __volatile__("cli");
    for(;;) { __asm__ __volatile__("hlt"); }
}

void paging_install(void) {
    page_directory = (page_directory_t*)pmm_alloc_page();
    memset(page_directory, 0, sizeof(page_directory_t));

    // Map Kernel (0-4MB)
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        paging_map(i, i, 0x3);
    }
    // Map Heap (4MB-20MB)
    for (uint32_t i = 0x00400000; i < 0x01400000; i += PAGE_SIZE) {
        paging_map(i, i, 0x3);
    }
    // Map Framebuffer
    uint32_t fb_phys = vbe_mode_info.physbase;
    if (fb_phys != 0) {
        for (uint32_t i = 0; i < 0x00800000; i += PAGE_SIZE) {
            paging_map(fb_phys + i, fb_phys + i, 0x3);
        }
    }
    register_interrupt_handler(14, page_fault_handler);
    switch_page_directory(page_directory);
}

// ==========================================
// FILE: timer.c
// ==========================================
volatile uint32_t ticks = 0;
void timer_handler(registers_t *r) {
    ticks++;
    schedule(r);
}
uint32_t get_ticks() {
    return ticks;
}
void timer_install() {
    int frequency = 100;
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);
    irq_install_handler(0, timer_handler); 
}

// ==========================================
// FILE: keyboard.c
// ==========================================
unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
    ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
    0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
unsigned char shift_kbdus[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0,
    ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0,
    0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int shift_pressed = 0;
static int caps_lock = 0;
static char key_buffer[256];
static int buffer_start = 0;
static int buffer_end = 0;

void keyboard_handler(registers_t *r) {
    (void)r;
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    if (scancode > 127) return;
    
    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == 42 || scancode == 54) shift_pressed = 0;
    } else {
        if (scancode == 42 || scancode == 54) {
            shift_pressed = 1;
        } else if (scancode == 58) {
            caps_lock = !caps_lock;
        } else {
            char ch = 0;
            if (scancode < 128) {
                if (shift_pressed) ch = shift_kbdus[scancode];
                else {
                    ch = kbdus[scancode];
                    if (caps_lock && ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
                }
            }
            if (ch != 0) {
                int next_end = (buffer_end + 1) % 256;
                if (next_end != buffer_start) {
                    key_buffer[buffer_end] = ch;
                    buffer_end = next_end;
                }
            }
        }
    }
}

char keyboard_getchar(void) {
    if (buffer_start == buffer_end) return 0;
    char ch = key_buffer[buffer_start];
    if (ch !=0){
        window_handle_key(ch);
        shell_handle_input(ch);
    }
    buffer_start = (buffer_start + 1) % 256;
    return ch;
}

void keyboard_wait_for_input(void) {
    while (buffer_start == buffer_end) {
        __asm__ __volatile__("hlt");
    }
}

void keyboard_install(void) {
    vga_print_string("Installing keyboard driver... ");
    buffer_start = 0;
    buffer_end = 0;
    shift_pressed = 0;
    caps_lock = 0;
    irq_install_handler(1, keyboard_handler);
    vga_print_string("[OK]\n");
}

// ==========================================
// FILE: mouse.c
// ==========================================
#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64
#define MOUSE_IRQ 12
volatile int32_t mouse_x = 400;
volatile int32_t mouse_y = 300;
volatile uint8_t mouse_buttons = 0;
mouse_packet_t mouse_packet;
static void mouse_handler(registers_t *regs);

void mouse_install(void) {
    outb(MOUSE_COMMAND_PORT, 0xA8);
    outb(MOUSE_COMMAND_PORT, 0x20);
    uint8_t status = inb(MOUSE_DATA_PORT);
    status |= 2; // Enable IRQ12
    outb(MOUSE_COMMAND_PORT, 0x60);
    outb(MOUSE_DATA_PORT, status);
    outb(MOUSE_COMMAND_PORT, 0xD4);
    outb(MOUSE_DATA_PORT, 0xF6);
    inb(MOUSE_DATA_PORT); 
    outb(MOUSE_COMMAND_PORT, 0xD4);
    outb(MOUSE_DATA_PORT, 0xF4);
    inb(MOUSE_DATA_PORT); 
    irq_install_handler(MOUSE_IRQ, mouse_handler);
}

static void mouse_handler(registers_t *regs) {
    (void)regs;
    static uint8_t packet_byte_index = 0;
    static mouse_packet_t packet;
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if (status & 0x20) {
        uint8_t data = inb(MOUSE_DATA_PORT);
        switch (packet_byte_index) {
            case 0: packet.flags = data; break;
            case 1: packet.x_delta = (int8_t)data; break;
            case 2: packet.y_delta = (int8_t)data; break;
        }
        packet_byte_index++;
        if (packet_byte_index == 3) {
            mouse_packet = packet;
            mouse_x += packet.x_delta;
            mouse_y -= packet.y_delta; // Invert Y axis
            mouse_buttons = packet.flags & 0x07;
            packet_byte_index = 0;
        }
    }
}

// ==========================================
// FILE: pci.c
// ==========================================
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

uint32_t pci_read_config(uint16_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_scan(void) {
    vga_print_string("Scanning PCI bus...\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t vendor_device_id = pci_read_config(bus, device, 0, 0);
            if ((vendor_device_id & 0xFFFF) != 0xFFFF) {
                uint16_t vendor = vendor_device_id & 0xFFFF;
                uint16_t device_id = vendor_device_id >> 16;
                vga_print_string("Found PCI device - Vendor: ");
                vga_print_hex(vendor);
                vga_print_string(", Device: ");
                vga_print_hex(device_id);
                vga_print_string("\n");
            }
        }
    }
}

// ==========================================
// FILE: nic.c
// ==========================================
#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139
#define REG_CONFIG_1    0x52
#define REG_COMMAND     0x37
#define REG_RX_BUF      0x30
#define REG_IMR         0x3C 
#define REG_ISR         0x3E 
#define REG_RCR         0x44 

uint32_t rtl8139_io_base = 0;
uint8_t* rx_buffer;

void rtl8139_handler(registers_t *r) {
    (void)r;
    vga_print_string("[NIC IRQ]");
    uint16_t status = inw(rtl8139_io_base + REG_ISR);
    if (status & 0x1) { 
        vga_print_string(" Packet Received ");
    }
    outw(rtl8139_io_base + REG_ISR, status);
}

void rtl8139_init(void) {
    vga_print_string("Initializing RTL8139... ");
    uint8_t found = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t vendor_device_id = pci_read_config(bus, device, 0, 0);
            if ((vendor_device_id & 0xFFFF) == RTL8139_VENDOR_ID && (vendor_device_id >> 16) == RTL8139_DEVICE_ID) {
                uint32_t bar0 = pci_read_config(bus, device, 0, 0x10);
                rtl8139_io_base = bar0 & 0xFFFC;
                found = 1;
                break;
            }
        }
        if (found) break;
    }

    if (!found) {
        vga_print_string("[NOT FOUND]\n");
        return;
    }
    vga_print_string("[OK]\n");
    outb(rtl8139_io_base + REG_CONFIG_1, 0x00);
    outb(rtl8139_io_base + REG_COMMAND, 0x10);
    while((inb(rtl8139_io_base + REG_COMMAND) & 0x10) != 0) { /* wait */ }
    rx_buffer = (uint8_t*)pmm_alloc_pages(4);
    outl(rtl8139_io_base + REG_RX_BUF, (uint32_t)rx_buffer);
    outw(rtl8139_io_base + REG_IMR, 0x0005);
    outl(rtl8139_io_base + REG_RCR, 0x0F);
    outb(rtl8139_io_base + REG_COMMAND, 0x0C);
    irq_install_handler(11, rtl8139_handler);
}

// ==========================================
// FILE: cursor.c
// ==========================================
#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
static const uint8_t cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH / 8] = {
    {0b00000000, 0b00000000}, {0b00000000, 0b10000000}, {0b00000001, 0b11000000}, {0b00000011, 0b11100000},
    {0b00000111, 0b11110000}, {0b00001111, 0b11111000}, {0b00011111, 0b11111100}, {0b00111111, 0b11111110},
    {0b01111111, 0b11111111}, {0b00111111, 0b11111110}, {0b00011111, 0b11111100}, {0b00001111, 0b11111000},
    {0b00000111, 0b11110000}, {0b00000011, 0b11100000}, {0b00000001, 0b11000000}, {0b00000000, 0b10000000}
};
static uint32_t cursor_bg_buffer[CURSOR_HEIGHT*CURSOR_WIDTH];
static uint32_t cursor_fg_buffer[CURSOR_HEIGHT*CURSOR_WIDTH];
static int prev_x = -1, prev_y = -1;
static int visible = 0;

void cursor_init(void) {
    prev_x = -1; prev_y = -1; visible = 0;
}
void cursor_draw(FrameBuffer* fb, int x, int y) {
    for (int cy = 0; cy < CURSOR_HEIGHT; cy++) {
        for (int cx = 0; cx < CURSOR_WIDTH; cx++) {
            int byte_index = cx / 8;
            uint8_t mask = 1 << (7 - (cx % 8));
            int fb_x = x + cx;
            int fb_y = y + cy;
            if (fb_x < 0 || fb_x >=(int)fb->width || fb_y < 0 || fb_y >= (int)fb->height) continue;
            uint32_t* fb_pixel = (uint32_t*)((uint8_t*)fb->address + (fb_y * fb->pitch) + (fb_x * (fb->bitsPerPixel / 8)));
            if (cursor_bitmap[cy][byte_index] & mask) {
                cursor_fg_buffer[cy * CURSOR_WIDTH + cx] = *fb_pixel;
                *fb_pixel = 0xFFFFFFFF; 
            } else {
                cursor_bg_buffer[cy * CURSOR_WIDTH + cx] = *fb_pixel;
                *fb_pixel = 0x00000000; 
            }
        }
    }
    visible = 1; prev_x = x; prev_y = y;
}
void cursor_update(FrameBuffer* fb, int x, int y) {
    if (visible) {
        dirty_rect_add(prev_x, prev_y, CURSOR_WIDTH, CURSOR_HEIGHT);
        for (int cy = 0; cy < CURSOR_HEIGHT; cy++) {
            for (int cx = 0; cx < CURSOR_WIDTH; cx++) {
                int fb_x = prev_x + cx;
                int fb_y = prev_y + cy;
                if (fb_x < 0 || fb_x >= (int)fb->width || fb_y < 0 || fb_y >= (int)fb->height) continue;
                uint32_t* fb_pixel = (uint32_t*)((uint8_t*)fb->address + (fb_y * fb->pitch) + (fb_x * (fb->bitsPerPixel / 8)));
                if (cursor_bitmap[cy][cx / 8] & (1 << (7 - (cx % 8)))) {
                    *fb_pixel = cursor_fg_buffer[cy * CURSOR_WIDTH + cx];
                } else {
                    *fb_pixel = cursor_bg_buffer[cy * CURSOR_WIDTH + cx];
                }
            }
        }
    }
    dirty_rect_add(x, y, CURSOR_WIDTH, CURSOR_HEIGHT);
    cursor_draw(fb, x, y);
}

// ==========================================
// FILE: dirty_rect.c
// ==========================================
static Rect dirty_rects[MAX_DIRTY_RECTS];
static int dirty_rect_count = 0;
static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }

void dirty_rect_init() {
    dirty_rect_count = 0;
}

void dirty_rect_add(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) return;
    Rect new_rect = {x, y, width, height};
restart_scan:
    for (int i = 0; i < dirty_rect_count; ++i) {
        Rect* existing = &dirty_rects[i];
        if (!(new_rect.x >= existing->x + existing->width ||
              new_rect.x + new_rect.width <= existing->x ||
              new_rect.y >= existing->y + existing->height ||
              new_rect.y + new_rect.height <= existing->y))
        {
            new_rect.x = min(existing->x, new_rect.x);
            new_rect.y = min(existing->y, new_rect.y);
            int x2 = max(existing->x + existing->width, new_rect.x + new_rect.width);
            int y2 = max(existing->y + existing->height, new_rect.y + new_rect.height);
            new_rect.width = x2 - new_rect.x;
            new_rect.height = y2 - new_rect.y;
            dirty_rects[i] = dirty_rects[dirty_rect_count - 1];
            dirty_rect_count--;
            goto restart_scan;
        }
    }
    if (dirty_rect_count < MAX_DIRTY_RECTS) {
        dirty_rects[dirty_rect_count++] = new_rect;
    } else {
        // Fallback: consolidate to 1 big rect if full
        for (int i = 1; i < dirty_rect_count; i++) {
             dirty_rect_add(dirty_rects[i].x, dirty_rects[i].y, dirty_rects[i].width, dirty_rects[i].height);
        }
        dirty_rect_count = 1;
    }
}

const Rect* dirty_rect_get_all(int* count) {
    *count = dirty_rect_count;
    return dirty_rects;
}

// ==========================================
// FILE: console.c
// ==========================================
static FrameBuffer* console_fb = NULL;
static Font* console_font = NULL;
void console_init(FrameBuffer* fb, Font* font) {
    console_fb = fb;
    console_font = font;
}
void console_write(const char* str) {
    if (!console_fb || !console_font) return;
    static uint32_t cursor_x = 0;
    static uint32_t cursor_y = 0;

    while (*str) {
        char c = *str++;
        if (c == '\n') {
            cursor_x = 0;
            cursor_y += console_font->char_height;
            if (cursor_y + console_font->char_height > console_fb->height) cursor_y = 0;
            continue;
        }
        draw_char(console_fb, console_font, c, cursor_x, cursor_y, 0xFFFFFF);
        cursor_x += console_font->char_width;
        if (cursor_x + console_font->char_width > console_fb->width) {
            cursor_x = 0;
            cursor_y += console_font->char_height;
        }
    }
}
void console_write_dec(uint32_t n) {
    if (n == 0) { console_write("0"); return; }
    char buf[11]; char* p = &buf[10]; *p = '\0';
    do { *--p = '0' + (n % 10); n /= 10; } while (n > 0);
    console_write(p);
}

// ==========================================
// FILE: syscall.c
// ==========================================
void syscalls_install(void){
	vga_print_string("Syscall Interface Installed, Simple OS WORKING\n");
}
void syscall_handler(registers_t *r){
	if (r->eax==0){
		vga_print_string("SYSTEM CALL RECEIVED!, SIMPLE OS WORKING\n");
	}
}

// ==========================================
// FILE: task.c
// ==========================================
static task_t* current_task;
static task_t* ready_queue;
static int next_pid = 1;

void idle_task_func(void){
    while(1) { __asm__ __volatile__("hlt"); }
}
task_t* get_current_task(void) {
    return current_task;
}
void tasking_install(void) {
    vga_print_string("Initializing multitasking... ");
    current_task = (task_t*)pmm_alloc_page();
    memset(current_task, 0, sizeof(task_t));
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->kernel_stack = NULL; 
    current_task->next = NULL;
    ready_queue = current_task; 
    vga_print_string("[OK]\n");
}
void schedule_from_yield(void){
    __asm__ __volatile__("int $0x20");
}
void create_task(char* name, void (*entry_point)(void)) {
    (void)name; 
    __asm__ __volatile__("cli");
    task_t* new_task = (task_t*)pmm_alloc_page();
    memset(new_task, 0, sizeof(task_t));

    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    new_task->kernel_stack = (void*)((uint32_t)pmm_alloc_page() + PAGE_SIZE);
    uint32_t stack_top = (uint32_t)new_task->kernel_stack;
    registers_t* initial_regs = (registers_t*)(stack_top - sizeof(registers_t));
    memset(initial_regs, 0, sizeof(registers_t));
    initial_regs->eip = (uint32_t)entry_point;
    initial_regs->eflags = 0x202; 
    new_task->regs.esp = (uint32_t)initial_regs;
    initial_regs->cs = 0x08; 
    initial_regs->ds = 0x10; 
    
    task_t* temp = ready_queue;
    while (temp->next != NULL) temp = temp->next;
    temp->next = new_task;
    new_task->next = ready_queue; 
    __asm__ __volatile__("sti");
}

void schedule(registers_t* r) {
    if (!current_task) return;
    current_task->regs = *r;
    task_t* next_task = current_task->next;
    while (next_task && next_task->state != TASK_READY) {
        next_task = next_task->next;
        if (next_task == current_task) {
            next_task = current_task;
            break;
        }
    }
    current_task = next_task;
    current_task->state = TASK_RUNNING;
    *r = current_task->regs;
}

void schedule_and_release_lock(spinlock_t* lock, unsigned long flags) {
    spinlock_release_irqrestore(lock, flags);
    schedule_from_yield();
}

// ==========================================
// FILE: widget.c
// ==========================================
Font* g_widget_font=NULL;
void widget_set_font(Font* font){ g_widget_font=font; }

void draw_label(Widget* self, FrameBuffer* fb){
    if(!g_widget_font)return;
    LabelData* data=(LabelData*)self->data;
    if(!data||!data->text)return;
    draw_string(fb, g_widget_font, data->text, self->x, self->y, data->color);
}
void update_label(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; }

void draw_button(Widget* self, FrameBuffer* fb){
    if(!g_widget_font)return;
    ButtonData* data=(ButtonData*)self->data;
    if(!data||!data->text)return;
    fill_rectangle(fb, self->x, self->y, self->width, self->height, data->bg_color);
    for(int i=0;i<data->border_width;i++){
        for(int y=i;y<self->height-i;y++){
            for(int x=i;x<self->width-i;x++){
                uint32_t color=data->border_color;
                if(x==i||x==self->width-i-1||y==i||y==self->height-i-1){
                    put_pixel(fb,self->x+x,self->y+y,color);
                }
            }
        }
    }
    int text_width=strlen(data->text)*g_widget_font->char_width;
    int text_height=g_widget_font->char_height;
    int text_x=self->x+self->width/2-text_width/2;
    int text_y=self->y+self->height/2-text_height/2;
    draw_string(fb, g_widget_font, data->text, text_x, text_y, data->text_color);
}
void update_button(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; }

void draw_textbox(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; if(!g_widget_font)return; }
void update_textbox(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; }
void draw_scrollbar(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; }
void update_scrollbar(Widget* self,FrameBuffer* fb){ (void)self; (void)fb; }

void handle_button_click(Widget* self,int mouse_x,int mouse_y,int button){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    if(button==1){ 
        data->bg_color=data->press_color;
        data->border_color=data->press_border;
    }
}
void handle_button_release(Widget* self,int mouse_x,int mouse_y,int button){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    if(button==1){ 
        data->bg_color=data->base_color;
        data->border_color=data->base_color;
    }
}
void handle_button_hover(Widget* self,int mouse_x,int mouse_y){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->hover_color;
    data->border_color=data->hover_border;
}
void handle_button_move(Widget* self,int mouse_x,int mouse_y){
    (void)self; (void)mouse_x; (void)mouse_y;
}
void handle_button_leave(Widget* self){
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->base_color;
    data->border_color=data->border_color;
}
void widget_draw(Widget* widget,FrameBuffer* fb){
    if(widget&&widget->draw) widget->draw(widget,fb);
}
void widget_update(Widget* widget,FrameBuffer* fb){
    if(widget&&widget->update) widget->update(widget,fb);
}
void widget_handle_event(Widget* widget,int mouse_x,int mouse_y,int event){
    if (!widget) return;
    if (mouse_x >= widget->x && mouse_x < widget->x + widget->width &&
        mouse_y >= widget->y && mouse_y < widget->y + widget->height) {
        
        if (event == EVENT_MOUSE_CLICK && widget->onClick) {
            widget->onClick(widget, mouse_x, mouse_y, 1);
        } else if (event == EVENT_MOUSE_RELEASE && widget->onRelease) {
            widget->onRelease(widget, mouse_x, mouse_y, 1); 
        } else if (event == EVENT_MOUSE_HOVER && widget->onHover) {
            widget->onHover(widget, mouse_x, mouse_y);
        } else if (event == EVENT_MOUSE_MOVE && widget->onMove) {
            widget->onMove(widget, mouse_x, mouse_y);
        }
    }
}
void widget_add(Widget** head,Widget* new_widget){
    if(!head||!new_widget)return;
    new_widget->next=*head;
    *head=new_widget;
}
void widget_remove(Widget** head,Widget* widget){
    if(!head||!widget)return;
    if(*head==widget){
        *head=widget->next;
        return;
    }
    Widget* current=*head;
    while(current&&current->next!=widget) current=current->next;
    if(current) current->next=widget->next;
}
void widget_free(Widget* widget){
    if(!widget)return;
    if(widget->data)free(widget->data);
    free(widget);
}
void widget_free_all(Widget** head){
    if(!head)return;
    Widget* current=*head;
    while(current){
        Widget* next=current->next;
        widget_free(current);
        current=next;
    }
    *head=NULL;
}
void widget_draw_all(Widget* head,FrameBuffer* fb){
    Widget* current=head;
    while(current){
        widget_draw(current,fb);
        current=current->next;
    }
}
void widget_update_all(Widget* head,FrameBuffer* fb){
    Widget* current=head;
    while(current){
        widget_update(current,fb);
        current=current->next;
    }
}
void widget_handle_event_all(Widget* head,int mouse_x,int mouse_y,int event){
    Widget* current=head;
    while(current){
        widget_handle_event(current,mouse_x,mouse_y,event);
        current=current->next;
    }
}
Widget* create_label(int x,int y,int width,int height,char* text,uint32_t color){
    LabelData* data=(LabelData*)malloc(sizeof(LabelData));
    if(!data)return NULL;
    data->text=text;
    data->color=color;
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){ free(data); return NULL; }
    widget->x=x; widget->y=y; widget->width=width; widget->height=height;
    widget->data=data;
    widget->draw=draw_label;
    widget->update=update_label;
    return widget;
}
Widget* create_button(int x,int y,int width,int height,char* text,uint32_t base_color,uint32_t hover_color,uint32_t press_color,uint32_t border_color,int border_width,uint32_t text_color,uint32_t press_border,uint32_t hover_border){
    ButtonData* data=(ButtonData*)malloc(sizeof(ButtonData));
    if(!data)return NULL;
    data->text=text; data->base_color=base_color; data->hover_color=hover_color;
    data->press_color=press_color; data->border_color=border_color; data->border_width=border_width;
    data->text_color=text_color; data->press_border=press_border; data->hover_border=hover_border;
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){ free(data); return NULL; }
    widget->x=x; widget->y=y; widget->width=width; widget->height=height;
    widget->data=data;
    widget->draw=draw_button; widget->update=update_button;
    widget->onClick=handle_button_click; widget->onRelease=handle_button_release; widget->onHover=handle_button_hover; widget->onMove=NULL;
    return widget;
}
Widget* create_textbox(int x,int y,int width,int height,char* placeholder,uint32_t bg_color,uint32_t text_color){
    TextboxData* data=(TextboxData*)malloc(sizeof(TextboxData));
    if(!data)return NULL;
    data->placeholder=placeholder; data->bg_color=bg_color; data->text_color=text_color;
    data->text=(char*)malloc(256); 
    if(!data->text){ free(data); return NULL; }
    data->text[0]='\0';
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){ free(data->text); free(data); return NULL; }
    widget->x=x; widget->y=y; widget->width=width; widget->height=height;
    widget->data=data; widget->draw=draw_textbox; widget->update=update_textbox;
    return widget;
}

typedef struct {
    uint32_t bg_color; uint32_t thumb_color; int thumb_pos; int thumb_size;
} ScrollbarData;
Widget* create_scrollbar(int x,int y,int width,int height,uint32_t bg_color,uint32_t thumb_color){
    ScrollbarData* data=(ScrollbarData*)malloc(sizeof(ScrollbarData));
    if(!data)return NULL;
    data->bg_color=bg_color; data->thumb_color=thumb_color; data->thumb_pos=0; data->thumb_size=height/4; 
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){ free(data); return NULL; }
    widget->x=x; widget->y=y; widget->width=width; widget->height=height;
    widget->data=data; widget->draw=draw_scrollbar; widget->update=update_scrollbar;
    return widget;
}
void init_widget_system(){
    if(!g_widget_font) vga_print_string("Failed to load widget font\n");
}

// ==========================================
// FILE: window.c
// ==========================================
#define WINDOW_BG_COLOR   0xECECEC   
#define TITLE_BAR_COLOR   0x2C2C2C   
#define TITLE_TEXT_COLOR  0xFFFFFF   
#define TITLE_BAR_HEIGHT  25 
#define CLOSE_BUTTON_WIDTH  18
#define CLOSE_BUTTON_HEIGHT 18
#define CLOSE_BUTTON_MARGIN 4
#define CLOSE_BUTTON_BG_COLOR 0xFF0000 
#define CLOSE_BUTTON_HOVER_BG_COLOR 0xFF4444 
#define CLOSE_BUTTON_X_COLOR  0xFFFFFF 

static spinlock_t wm_lock;
static Window* focused_window = NULL;

Window* create_window(int x,int y,int width,int height,const char* title,bool has_title_bar){
    Window* window=(Window*)malloc(sizeof(Window));
    if(!window)return NULL;
    window->x=x; window->y=y; window->width=width; window->height=height;
    window->title=title; window->has_title_bar=has_title_bar;
    window->child_widgets_head=NULL; window->child_widgets_tail=NULL;
    window->next=NULL; window->prev=NULL; window->close_button_hovered = false;
    return window;
}
void window_add_widget(Window* window,Widget* widget){
    if(!window||!widget)return;
    if(!window->child_widgets_head){
        window->child_widgets_head=widget;
        window->child_widgets_tail=widget;
    }else{
        window->child_widgets_tail->next=widget;
        window->child_widgets_tail=widget;
    }
    widget->next=NULL; 
}
void window_remove_widget(Window* window,Widget* widget){
    if(!window||!widget||!window->child_widgets_head)return;
    if(window->child_widgets_head==widget){
        window->child_widgets_head=widget->next;
        if(window->child_widgets_tail==widget) window->child_widgets_tail=NULL;
        return;
    }
    Widget* current=window->child_widgets_head;
    while(current&&current->next!=widget) current=current->next;
    if(current){
        current->next=widget->next;
        if(window->child_widgets_tail==widget) window->child_widgets_tail=current;
    }
}
void window_free_widgets(Window* window){
    if(!window)return;
    Widget* current=window->child_widgets_head;
    while(current){
        Widget* next=current->next;
        if(current->data)free(current->data);
        free(current);
        current=next;
    }
    window->child_widgets_head=NULL;
    window->child_widgets_tail=NULL;
}
void window_free(Window* window){
    if(!window)return;
    window_free_widgets(window);
    free(window);
}
void window_draw(Window* window,FrameBuffer* fb){
    if(!window)return;
    fill_rectangle(fb, window->x, window->y, window->width, window->height, WINDOW_BG_COLOR);

    if(window->has_title_bar&&window->title){
        fill_rectangle(fb, window->x, window->y, window->width, TITLE_BAR_HEIGHT, TITLE_BAR_COLOR);
        if(g_widget_font){
            int text_width=strlen(window->title)*g_widget_font->char_width;
            int text_height=g_widget_font->char_height;
            int text_x=window->x+window->width/2-text_width/2;
            int text_y=window->y+(TITLE_BAR_HEIGHT-text_height)/2;
            draw_string(fb,g_widget_font,window->title,text_x,text_y,TITLE_TEXT_COLOR);
        }
        int btn_x = window->x + window->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
        int btn_y = window->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;
        uint32_t btn_color = window->close_button_hovered ? CLOSE_BUTTON_HOVER_BG_COLOR : CLOSE_BUTTON_BG_COLOR;
        fill_rectangle(fb, btn_x, btn_y, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, btn_color);
        int x_margin = 4;
        draw_line(fb, btn_x + x_margin, btn_y + x_margin, 
                      btn_x + CLOSE_BUTTON_WIDTH - x_margin - 1, btn_y + CLOSE_BUTTON_HEIGHT - x_margin - 1, 
                      CLOSE_BUTTON_X_COLOR);
        draw_line(fb, btn_x + x_margin, btn_y + CLOSE_BUTTON_HEIGHT - x_margin - 1,
                      btn_x + CLOSE_BUTTON_WIDTH - x_margin - 1, btn_y + x_margin,
                      CLOSE_BUTTON_X_COLOR);
    }
    widget_draw_all(window->child_widgets_head,fb);
}
void window_update(Window* window,FrameBuffer* fb){
    if(!window)return;
    widget_update_all(window->child_widgets_head,fb);
}
void window_handle_event(Window* window,int mouse_x,int mouse_y,int event){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,event);
}
void window_on_click(Window* window,int mouse_x,int mouse_y,int button){
    (void)button;
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,EVENT_MOUSE_CLICK);
}
void window_on_release(Window* window,int mouse_x,int mouse_y,int button){
    (void)button;
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,EVENT_MOUSE_RELEASE);
}
void window_on_hover(Window* window,int mouse_x,int mouse_y){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,EVENT_MOUSE_HOVER);
}
void window_on_move(Window* window,int mouse_x,int mouse_y){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,EVENT_MOUSE_MOVE);
}

void window_manager_init() {
    spinlock_init(&wm_lock);
}

void window_destroy(Window** head, Window** tail, Window* win_to_destroy) {
    if (!win_to_destroy) return;
    spinlock_acquire(&wm_lock);
    if (win_to_destroy->prev) win_to_destroy->prev->next = win_to_destroy->next;
    else *head = win_to_destroy->next;
    if (win_to_destroy->next) win_to_destroy->next->prev = win_to_destroy->prev;
    else *tail = win_to_destroy->prev;
    spinlock_release(&wm_lock);
    window_free_widgets(win_to_destroy);
    free(win_to_destroy);
}

void window_bring_to_front(Window** head, Window** tail, Window* win) {
    if (!win || *tail == win) return;
    spinlock_acquire(&wm_lock);
    if (win->prev) win->prev->next = win->next;
    else *head = win->next;
    if (win->next) win->next->prev = win->prev;
    else *tail = win->prev;
    (*tail)->next = win;
    win->prev = *tail;
    win->next = NULL;
    *tail = win;
    spinlock_release(&wm_lock);
}

static Window* dragged_window = NULL;
static int32_t drag_offset_x = 0;
static int32_t drag_offset_y = 0;

void window_manager_handle_mouse(Window** head, Window** tail, int32_t mouse_x, int32_t mouse_y, uint8_t mouse_buttons, uint8_t last_buttons) {
    spinlock_acquire(&wm_lock);
    Window* top_win = *tail;
    if (top_win) {
        int btn_x = top_win->x + top_win->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
        int btn_y = top_win->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;
        if (mouse_x >= btn_x && mouse_x < btn_x + CLOSE_BUTTON_WIDTH &&
            mouse_y >= btn_y && mouse_y < btn_y + CLOSE_BUTTON_HEIGHT) {
            top_win->close_button_hovered = true;
        } else {
            top_win->close_button_hovered = false;
        }
    }

    if ((mouse_buttons & 1) && !(last_buttons & 1)) {
        Window* win = *tail;
        if (win) { 
            int btn_x = win->x + win->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
            int btn_y = win->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;
            if (mouse_x >= btn_x && mouse_x < btn_x + CLOSE_BUTTON_WIDTH &&
                mouse_y >= btn_y && mouse_y < btn_y + CLOSE_BUTTON_HEIGHT) {
                
                window_destroy(head, tail, win); 
                spinlock_release(&wm_lock);      
                return;                          

            } else if (mouse_x >= win->x && mouse_x < win->x + win->width &&
                       mouse_y >= win->y && mouse_y < win->y + TITLE_BAR_HEIGHT) {
                dragged_window = win;
                drag_offset_x = mouse_x - win->x;
                drag_offset_y = mouse_y - win->y;
                window_bring_to_front(head, tail, win);
            }
        }
    }

    if (!(mouse_buttons & 1) && (last_buttons & 1)) dragged_window = NULL;

    if (dragged_window != NULL) {
        dirty_rect_add(dragged_window->x, dragged_window->y, dragged_window->width, dragged_window->height);
        dragged_window->x = mouse_x - drag_offset_x;
        dragged_window->y = mouse_y - drag_offset_y;
    }
    spinlock_release(&wm_lock);
}
void wm_process_mouse(int mouse_x,int mouse_y,uint8_t mouse_buttons,uint8_t last_buttons){
    window_manager_handle_mouse(&window_list_head,&window_list_tail,mouse_x,mouse_y,mouse_buttons,last_buttons);
}
void window_handle_key(char key){
    (void)key;
    if(!window_list_tail)return;
}
void window_draw_all(Window* head,FrameBuffer* fb){
    spinlock_acquire(&wm_lock);
    Window* current=head;
    while(current){
        window_draw(current,fb);
        current=current->next;
    }
    spinlock_release(&wm_lock);
}
void window_set_focus(Window* window){
    if(!window)return;
    spinlock_acquire(&wm_lock);
    window_bring_to_front(&window_list_head,&window_list_tail,window);
    spinlock_release(&wm_lock);
    focused_window=window;
}
Window* wm_get_focused_window(){ return focused_window; }
void window_update_all(Window* head,FrameBuffer* fb){
    spinlock_acquire(&wm_lock);
    Window* current=head;
    while(current){
        window_update(current,fb);
        current=current->next;
    }
    spinlock_release(&wm_lock);
}

// ==========================================
// FILE: shell.c
// ==========================================
#define MAX_COMMAND_LENGTH 256
static char command_buffer[MAX_COMMAND_LENGTH];
static int command_index = 0;

static void shell_clear_screen(void) {
    vga_clear();
    vga_print_string("SimpleOS Shell\n");
    vga_print_string("==============\n\n");
}

static void shell_help(void) {
    vga_print_string("\nAvailable commands:\n");
    vga_print_string("  help    - Show this help message\n");
    vga_print_string("  clear   - Clear the screen\n");
    vga_print_string("  about   - Show system information\n");
    vga_print_string("  reboot  - Reboot the system\n");
    vga_print_string("  halt    - Halt the system\n\n");
}

static void shell_about(void) {
    vga_print_string("\nSimpleOS v1.0\n");
    vga_print_string("A basic x86 operating system kernel\n");
    vga_print_string("Features: GDT, IDT, VGA driver, Keyboard input\n");
}

static void shell_reboot(void) {
    vga_print_string("Rebooting system...\n");
    uint8_t good = 0x02;
    while (good & 0x02) good = inb(0x64);
    outb(0x64, 0xFE);
    asm volatile("hlt");
}

static void shell_halt(void) {
    vga_print_string("System halted. You may now turn off your computer.\n");
    asm volatile("cli");
    for(;;) { asm volatile("hlt"); }
}

static int shell_strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) { str1++; str2++; }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

static void shell_execute_command(void) {
    if (command_index == 0) {
        shell_print_prompt();
        return;
    }
    
    if (command_index >= MAX_COMMAND_LENGTH) command_index = MAX_COMMAND_LENGTH - 1;
    command_buffer[command_index] = '\0';
    
    if (shell_strcmp(command_buffer, "help") == 0) shell_help();
    else if (shell_strcmp(command_buffer, "clear") == 0) shell_clear_screen();
    else if (shell_strcmp(command_buffer, "about") == 0) shell_about();
    else if (shell_strcmp(command_buffer, "reboot") == 0) shell_reboot();
    else if (shell_strcmp(command_buffer, "halt") == 0) shell_halt();
    else if (shell_strcmp(command_buffer, "time") == 0){
        uint32_t ticks = get_ticks()/500;
        vga_print_string("Ticks: ");
        vga_print_dec(ticks);
        vga_print_string("Seconds\n");
    }
    else {
        vga_print_string("\nUnknown command: ");
        vga_print_string(command_buffer);
        vga_print_string("\nType 'help' for available commands.\n\n");
    }
    
    command_index = 0;
    shell_print_prompt();
}

void shell_print_prompt(void) {
    vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vga_print_string("SimpleOS> ");
    vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void shell_handle_input(char ch) {
    if (ch == '\n') {
        vga_putchar('\n');
        shell_execute_command();
    } else if (ch == '\b') {
        if (command_index > 0) {
            command_index--;
            command_buffer[command_index] = '\0';
            vga_putchar('\b');
            vga_putchar(' ');
            vga_putchar('\b');
        }
    } else if (ch >= 32 && ch <= 126) {
        if (command_index < MAX_COMMAND_LENGTH - 2) {
            command_buffer[command_index++] = ch;
            vga_putchar(ch);
        } else {
            vga_print_string("\nCommand too long! Maximum length is ");
            vga_print_dec(MAX_COMMAND_LENGTH - 1);
            vga_print_string(" characters.\n");
            command_index = 0;
            shell_print_prompt();
        }
    }
}

void shell_init(void) {
    command_index = 0;
    shell_clear_screen();
    vga_print_string("Welcome to SimpleOS!\n");
    vga_print_string("Type 'help' for available commands.\n\n");
    shell_print_prompt();
}

// ==========================================
// FILE: mutex.c
// ==========================================

void mutex_init(mutex_t* mutex) {
    if (mutex == NULL) return;
    spinlock_init(&mutex->lock);
    mutex->locked = false;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
}

void mutex_lock(mutex_t* mutex) {
    if (mutex == NULL) return;
    unsigned long flags = spinlock_acquire_irqsave(&mutex->lock);

    task_t* current = get_current_task();
    if (mutex->locked && mutex->owner != current) {
        current->state = TASK_BLOCKED;
        current->next = mutex->wait_queue;
        mutex->wait_queue = current;
        schedule_and_release_lock(&mutex->lock, flags);
    } else {
        mutex->locked = true;
        mutex->owner = current;
        spinlock_release_irqrestore(&mutex->lock, flags);
    }
}

void mutex_unlock(mutex_t* mutex) {
    if (mutex == NULL) return;
    unsigned long flags = spinlock_acquire_irqsave(&mutex->lock);
    mutex->locked = false;
    mutex->owner = NULL;
    if (mutex->wait_queue != NULL) {
        task_t* next_task = mutex->wait_queue;
        mutex->wait_queue = next_task->next;
        next_task->state = TASK_READY;
    }
    spinlock_release_irqrestore(&mutex->lock, flags);
}

// ==========================================
// FILE: main kernel.c
// ==========================================
#define VBE_INFO_PTR ((VbeModeInfo*)0x8000)

void counter_task(){
    int i=0;
    while(1){
        vga_putentryat('A' + (i++%26),0x0F,79,0);
    }
}

void on_my_button_click(Widget* self, int x, int y, int button){
    (void)x; (void)y; (void)button;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->press_color;
    data->border_color=data->press_border;
}

void on_my_button_release(Widget* self, int x, int y, int button){
    (void)x; (void)y; (void)button;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->base_color;
    data->border_color=data->base_color;
}

void kernel_main(void) {
    gdt_install();
    idt_install();
    pmm_init(128 * 1024 * 1024);
    paging_install();
    heap_init(0x00400000, 16 * 1024 * 1024); // 16 MB heap at 4 MB
    memcpy(&vbe_mode_info, (void*)0x8000, sizeof(VbeModeInfo));
    FrameBuffer fb;
    fb.address=(void*)vbe_mode_info.physbase;
    fb.width=vbe_mode_info.resolution_x;
    fb.height=vbe_mode_info.resolution_y;
    fb.pitch=vbe_mode_info.pitch;
    fb.bitsPerPixel=vbe_mode_info.bitsPerPixel;
    fb.bytesPerPixel = vbe_mode_info.bitsPerPixel / 8;
    
    Font my_font;
    my_font.char_width=8;
    my_font.char_height=16;
    my_font.bitmap=font; // Use the font array from font.c (Section 2)
    
    console_init(&fb, &my_font);

    console_write("SimpleOS Kernel v1.0 [SECURED]\n");
    console_write("===============================\n\n");
    
    syscalls_install();
    timer_install();
    keyboard_install();
    __asm__ __volatile__("sti");
    rtl8139_init();
    tasking_install();
    mouse_install();
    cursor_init();

    uint32_t free_mem = pmm_get_free_memory();
    console_write("Free memory: ");
    console_write_dec(free_mem / 1024);
    console_write(" KB\n");
    
    console_write("\nKernel initialization complete!\n");
    console_write("Starting GUI...\n\n");

    // --- GUI INITIALIZATION ---

    // 1. Initialize Managers & Fonts FIRST
    init_widget_system();
    window_manager_init();
    dirty_rect_init();
    widget_set_font(&my_font); 

    // 2. Clear Screen
    clear_screen(&fb, 0xECECEC); 

    // 3. Create Window
    Window* main_window = create_window(100, 100, 400, 300, "Welcome to SimpleOS!", true);
    if (!main_window) {
        console_write("Error: Failed to create main window!\n"); 
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    // 4. Create Widgets
    Widget* label = create_label(20, 40, 200, 30, "Hello, SimpleOS!", 0x000000);
    if (!label) {
        console_write("Error: Failed to create label widget!\n"); 
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    Widget* button = create_button(100, 100, 100, 50, "Click Me", 0xFFFFFF, 0x0000FF, 0x000000, 0x000000, 1, 0x000000, 0x000000, 0x000000);
    if (!button) {
        console_write("Error: Failed to create button widget!\n"); 
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    // 5. Assemble UI
    window_add_widget(main_window, label);
    window_add_widget(main_window, button);
    
    button->onClick = on_my_button_click;
    button->onRelease = on_my_button_release;
    
    window_list_head = main_window;
    window_list_tail = main_window;

    // 6. Initial Draw
    window_draw(main_window, &fb);
    dirty_rect_add(0, 0, fb.width, fb.height);

    uint8_t last_buttons = 0;
    int32_t last_x = -1, last_y = -1; 

    // Main Loop
    while(1) {
        bool mouse_moved = (mouse_x != last_x || mouse_y != last_y);
        bool buttons_changed = (mouse_buttons != last_buttons);

        if (mouse_moved || buttons_changed) {
            // Redraw only affected areas
            dirty_rect_add(last_x, last_y, 16, 16); // Old cursor pos
            window_manager_handle_mouse(&window_list_head, &window_list_tail, mouse_x, mouse_y, mouse_buttons, last_buttons);
            dirty_rect_add(mouse_x, mouse_y, 16, 16); // New cursor pos

            last_x = mouse_x;
            last_y = mouse_y;
            last_buttons = mouse_buttons;
        }

        int dirty_count;
        const Rect* rects = dirty_rect_get_all(&dirty_count);

        if (dirty_count > 0) {
            for (int i = 0; i < dirty_count; i++) {
                const Rect* dirty = &rects[i];
                Window* current = window_list_head;
                while (current) {
                    // Simple AABB collision check to see if window needs update
                    if (!(current->x > dirty->x + dirty->width || current->x + current->width < dirty->x ||
                          current->y > dirty->y + dirty->height || current->y + current->height < dirty->y)) {
                        window_draw(current, &fb); 
                    }
                    current = current->next;
                }
            }
            dirty_rect_init();
        }

        cursor_update(&fb, mouse_x, mouse_y);
        __asm__ __volatile__("hlt");
    }
}