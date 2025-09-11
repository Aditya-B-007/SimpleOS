#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "idt.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void keyboard_install(void);
void keyboard_handler(registers_t *r);
char keyboard_getchar(void);
void keyboard_wait_for_input(void);

extern unsigned char kbdus[128];

#endif