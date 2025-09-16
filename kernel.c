#include "vga.h"
#include "idt.h"
#include "gdt.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
//#include "pci.h"
#include "NIC.h"
#include "pmm.h"

void kernel_main(void) {
    vga_init();
    vga_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    vga_print_string("SimpleOS Kernel v1.0 [SECURED]\n");
    vga_print_string("===============================\n\n");
    
    vga_print_string("Initializing PMM... ");
    pmm_init(128*1024*1024);
    vga_print_string("[OK]\n");
    
    vga_print_string("Initializing GDT... ");
    gdt_install();
    vga_print_string("[OK]\n");
    
    vga_print_string("Initializing IDT... ");
    idt_install();
    vga_print_string("[OK]\n");

    paging_install();
    vga_print_string("Initializing PMM... ");
    pmm_init(128 * 1024 * 1024);
    
    vga_print_string("Setting up interrupts... ");
    asm volatile("sti");
    vga_print_string("[OK]\n");
    
    keyboard_install();
    timer_install();
    //pci_scan();
    rtl8139_init();
    
    uint32_t free_mem = pmm_get_free_memory();
    vga_print_string("Free memory: ");
    vga_print_dec(free_mem / 1024);
    vga_print_string(" KB\n");
    
    vga_print_string("\nKernel initialization complete!\n");
    vga_print_string("Security features: Buffer overflow protection, bounds checking\n");
    vga_print_string("Starting shell...\n\n");
    
    shell_init();
    
    while(1) {
        asm volatile("hlt");
    }
}
