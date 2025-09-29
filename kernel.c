#include <stdint.h>
#include "vga.h"
#include "idt.h"
#include "gdt.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "paging.h"
//#include "pci.h"
#include "NIC.h"
#include "pmm.h"
#include "task.h"
#include "syscall.h"
void counter_task(){
    int i=0;
    while(1){
        vga_putentryat('A' + (i++%26),0x0F,79,0);
    }
}
void kernel_main(void) {
    vga_init();
    vga_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    vga_print_string("SimpleOS Kernel v1.0 [SECURED]\n");
    vga_print_string("===============================\n\n");
    gdt_install();
    idt_install();
    syscalls_install();
    paging_install();
    pmm_init(128 * 1024 * 1024);
    asm volatile("sti");
    keyboard_install();
    timer_install();
    rtl8139_init();
    tasking_install();
    create_task("shell",shell_init);
    create_task("counter",counter_task);
    uint32_t free_mem = pmm_get_free_memory();
    vga_print_dec(free_mem / 1024);
    vga_print_string(" KB\n");
    
    vga_print_string("\nKernel initialization complete!\n");
    vga_print_string("Security features: Buffer overflow protection, bounds checking\n");
    vga_print_string("Starting shell...\n\n");
    
    shell_init();

    // Main kernel loop - wait for interrupts
    while(1) {
        asm volatile("hlt");
    }
}
