#include "shell.h"
#include "keyboard.h"
#include "vga.h"
#include <stdint.h>
#include "timer.h"

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
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    asm volatile("hlt");
}

static void shell_halt(void) {
    vga_print_string("System halted. You may now turn off your computer.\n");
    asm volatile("cli");
    for(;;) {
        asm volatile("hlt");
    }
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static int shell_strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

static void shell_execute_command(void) {
    if (command_index == 0) {
        shell_print_prompt();
        return;
    }
    
    if (command_index >= MAX_COMMAND_LENGTH) {
        command_index = MAX_COMMAND_LENGTH - 1;
    }
    command_buffer[command_index] = '\0';
    
    if (shell_strcmp(command_buffer, "help") == 0) {
        shell_help();
    } else if (shell_strcmp(command_buffer, "clear") == 0) {
        shell_clear_screen();
    } else if (shell_strcmp(command_buffer, "about") == 0) {
        shell_about();
    } else if (shell_strcmp(command_buffer, "reboot") == 0) {
        shell_reboot();
    } else if (shell_strcmp(command_buffer, "halt") == 0) {
        shell_halt();
    } 
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