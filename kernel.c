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
#include "graphics.h"
#include "widget.h"
#include "font.h"
#include <stddef.h>
#include "NIC.h"
#include "pmm.h"
#include "task.h"
#include "syscall.h"
#include "window.h"
void counter_task(){
    int i=0;
    while(1){
        vga_putentryat('A' + (i++%26),0x0F,79,0);
    }
}
Widget* widget_list_head=NULL;
Window* window_list_head=NULL;
Window* window_list_tail=NULL;
void on_my_button_click(){
    if(!g_widget_font)return;
    ButtonData* data=(ButtonData*)widget_list_head->data;
    if(!data)return;
    data->bg_color=data->press_color;
    data->border_color=data->press_border;
    //vga_print_string("Button clicked!\n");
}
void on_my_button_release(){
    if(!g_widget_font)return;
    ButtonData* data=(ButtonData*)widget_list_head->data;
    if(!data)return;
    data->bg_color=data->base_color;
    data->border_color=data->border_color;
    //vga_print_string("Button released!\n");
}
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
    Font my_font;
    my_font.char_width=8;
    my_font.char_height=16;
    my_font.bitmap=NULL; // Assume a function to load a bitmap font

    if (fb.bitsPerPixel != 24) {
        vga_print_string("Error: Graphics mode is not 24-bit color depth!\n");
        for(;;) { asm volatile("cli; hlt"); }
    }
    uint32_t xc = fb.width / 2;
    uint32_t yc = fb.height / 2;

    clear_screen(&fb, 0x000000);
    draw_circle(&fb, xc, yc, 50,  0xFFFFFF);
    Window* main_window = create_window(100, 100, 400, 300, (char*[]){"Main Window"}, true);
    if (!main_window) {
        vga_print_string("Error: Failed to create main window!\n");
        for(;;) { asm volatile("cli; hlt"); }
    }
    Widget* label = create_label(20, 40, 200, 30, "Hello, SimpleOS!", 0x000000);
    if (!label) {
        vga_print_string("Error: Failed to create label widget!\n");
        for(;;) { asm volatile("cli; hlt"); }
    }
    Widget* button = create_button(100, 100, 100, 50, "Click Me", 0x0000FF, 0x00FF00, 0xFF0000, 0x000000, 2, 0xFFFFFF, 0xFFFF00, 0x00FFFF);
    if (!button) {
        vga_print_string("Error: Failed to create button widget!\n");
        for(;;) { asm volatile("cli; hlt"); }
    }
    widget_add(&widget_list_head, label);
    widget_add(&widget_list_head, button);
    window_add_widget(main_window, label);
    window_add_widget(main_window, button);
    window_draw(main_window, &fb);
    button->onClick = (void (*)(struct Widget*, int, int, int))on_my_button_click;
    button->onRelease = (void (*)(struct Widget*, int, int, int))on_my_button_release;
    window_list_head = main_window;
    window_list_tail = main_window;
    init_widget_system();
    widget_set_font(&my_font);
    //Scheduler takes over, chillax!!
    while(1) {
        asm volatile("hlt");
    }
}
