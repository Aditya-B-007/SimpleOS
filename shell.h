#ifndef SHELL_H
#define SHELL_H

#include "vga.h"
#include <stdint.h>

void shell_init(void);
void shell_handle_input(char ch);
void shell_print_prompt(void);

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

#endif