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
#include "mouse.h"
#include "cursor.h"
#include "heap.h"
#include "console.h"
#include "dirty_rect.h"
#define VBE_INFO_PTR ((vbe_mode_info_t*)0x8000)
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
    ButtonData* data=(ButtonData*)widget_list_head->data;
    if(!data)return;
    data->bg_color=data->press_color;
    data->border_color=data->press_border;
}
void on_my_button_release(){
    ButtonData* data=(ButtonData*)widget_list_head->data;
    if(!data)return;
    data->bg_color=data->base_color;
    data->border_color=data->border_color;
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
    uint32_t physbase;
} __attribute__((packed)) vbe_mode_info_t;

extern vbe_mode_info_t vbe_mode_info;

void kernel_main(void) {
    gdt_install();
    idt_install();
    pmm_init(128 * 1024 * 1024);
    heap_init(0x00400000, 16 * 1024 * 1024); // 16 MB heap at 4 MB
    vga_init();
    vga_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    vga_print_string("SimpleOS Kernel v1.0 [SECURED]\n");
    vga_print_string("===============================\n\n");
    syscalls_install();
    paging_install();
    console_init();
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
    console_init_graphics(&fb,&my_font);
    asm volatile("sti");
    keyboard_install();
    timer_install();
    rtl8139_init();
    tasking_install();
    mouse_install();
    cursor_init();
    create_task("shell",shell_init);
    create_task("counter",counter_task);
    uint32_t free_mem = pmm_get_free_memory();
    vga_print_dec(free_mem / 1024);
    vga_print_string(" KB\n");
    
    vga_print_string("\nKernel initialization complete!\n");
    vga_print_string("Security features: Buffer overflow protection, bounds checking\n");
    vga_print_string("Starting shell...\n\n");
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
    window_manager_init();
    dirty_rect_init();
    widget_set_font(&my_font);
    //Scheduler takes over, chillax!!

    #define CURSOR_TRAIL_LENGTH 8
    struct { int32_t x, y; } cursor_history[CURSOR_TRAIL_LENGTH] = {0};
    int history_index = 0;
    uint8_t last_buttons = 0;
    int32_t last_x = -1, last_y = -1; 
    dirty_rect_add(0, 0, fb.width, fb.height);

    while(1) {
        bool mouse_moved = (mouse_x != last_x || mouse_y != last_y);
        bool buttons_changed = (mouse_buttons != last_buttons);

        if (mouse_moved || buttons_changed) {
            dirty_rect_add(last_x, last_y, 16, 16); 
            window_manager_handle_mouse(&window_list_head, &window_list_tail, mouse_x, mouse_y, mouse_buttons, last_buttons);
            dirty_rect_add(mouse_x, mouse_y, 16, 16);

            last_x = mouse_x;
            last_y = mouse_y;
            last_buttons = mouse_buttons;
        }
        int dirty_count;
        const Rect* rects = dirty_rect_get_all(&dirty_count);

        if (dirty_count > 0) {
            for (int i = 0; i < dirty_count; i++) {
                const Rect* dirty = &rects[i];
                Window* current = window_list_head;
                while (current) {
                    if (!(current->x > dirty->x + dirty->width || current->x + current->width < dirty->x ||
                          current->y > dirty->y + dirty->height || current->y + current->height < dirty->y)) {
                        window_draw(current, &fb); 
                    }
                    current = current->next;
                }
            }
            dirty_rect_init();
        }

        cursor_update(&fb, mouse_x, mouse_y);

        asm volatile("hlt");
    }
}
