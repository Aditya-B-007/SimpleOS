#include <stdint.h>
#include "graphics.h"
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

// Assuming vbe_mode_info is provided by the bootloader.
// This is a common structure for VBE mode information.
typedef struct {
    uint16_t attributes;
    uint8_t window_a, window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a, segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch, resolution_x, resolution_y;
    uint8_t chars_x, chars_y, planes, bitsPerPixel, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    // ... other fields might follow
    uint32_t physbase;
} __attribute__((packed)) vbe_mode_info_t;

extern vbe_mode_info_t vbe_mode_info;

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
    FrameBuffer fb;
    fb.address=(void*)vbe_mode_info.physbase;
    fb.width=vbe_mode_info.resolution_x;
    fb.height=vbe_mode_info.resolution_y;
    fb.pitch=vbe_mode_info.pitch;
    fb.bitsPerPixel=vbe_mode_info.bitsPerPixel;
    fb.bytesPerPixel = vbe_mode_info.bitsPerPixel / 8;
    if (fb.bitsPerPixel != 24) {
        vga_print_string("Error: Graphics mode is not 24-bit color depth!\n");
        for(;;) { asm volatile("cli; hlt"); }
    }
    uint32_t xc = fb.width / 2;
    uint32_t yc = fb.height / 2;

    clear_screen(&fb, 0x000000);
    draw_circle(&fb, xc, yc, 50,  0xFFFFFF);
    //Scheduler takes over, chillax!!
    while(1) {
        asm volatile("hlt");
    }
}
