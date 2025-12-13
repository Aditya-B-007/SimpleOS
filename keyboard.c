#include "keyboard.h"
#include "idt.h"
#include "vga.h"
#include "shell.h"
#include <stdint.h>
#include "window.h"
#include "io.h"

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
    
    if (scancode > 127) {
        return;
    }
    
    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == 42 || scancode == 54) {
            shift_pressed = 0;
        }
    } else {
        if (scancode == 42 || scancode == 54) {
            shift_pressed = 1;
        } else if (scancode == 58) {
            caps_lock = !caps_lock;
        } else {
            char ch = 0;
            
            if (scancode < 128) {
                if (shift_pressed) {
                    ch = shift_kbdus[scancode];
                } else {
                    ch = kbdus[scancode];
                    if (caps_lock && ch >= 'a' && ch <= 'z') {
                        ch = ch - 'a' + 'A';
                    }
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
    if (buffer_start == buffer_end) {
        return 0;
    }
    
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
        asm volatile("hlt");
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