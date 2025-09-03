#include "vga.h"

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