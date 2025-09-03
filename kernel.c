#include "vga.h"
#include "idt.h"
#include "gdt.h"

void kernel_main(void) {
    vga_init();
    vga_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    vga_print_string("SimpleOS Kernel v1.0\n");
    vga_print_string("====================\n\n");
    
    vga_print_string("Initializing GDT... ");
    gdt_install();
    vga_print_string("[OK]\n");
    
    vga_print_string("Initializing IDT... ");
    idt_install();
    vga_print_string("[OK]\n");
    
    vga_print_string("Setting up interrupts... ");
    asm volatile("sti");
    vga_print_string("[OK]\n\n");
    
    vga_print_string("Kernel initialization complete!\n");
    vga_print_string("System ready for operation.\n\n");
    
    vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vga_print_string("Welcome to SimpleOS!\n");
    
    vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    while(1) {
        asm volatile("hlt");
    }
}
