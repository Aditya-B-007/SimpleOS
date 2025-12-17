#include <stdint.h>
#include <string.h>
#include "graphics.h"
#include "vga.h"
#include "idt.h"
#include "gdt.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "paging.h"
//#include "pci.h"
#include "widget.h"
#include "font.h"
#include <stddef.h>
#include "nic.h"
#include "pmm.h"
#include "task.h"
#include "syscall.h"
#include "window.h"
#include "mouse.h"
#include "cursor.h"
#include "heap.h"
#include "console.h"
#include "dirty_rect.h"
#include <stdbool.h>
#define VBE_INFO_PTR ((VbeModeInfo*)0x8000)
void counter_task(){
    int i=0;
    while(1){
        vga_putentryat('A' + (i++%26),0x0F,79,0);
    }
}
Widget* widget_list_head=NULL;
Window* window_list_head=NULL;
Window* window_list_tail=NULL;
void on_my_button_click(Widget* self, int x, int y, int button){
    (void)x; (void)y; (void)button;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->press_color;
    data->border_color=data->press_border;
}
void on_my_button_release(Widget* self, int x, int y, int button){
    (void)x; (void)y; (void)button;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    data->bg_color=data->base_color;
    data->border_color=data->base_color;
}

VbeModeInfo vbe_mode_info = {0};

void kernel_main(void) {
    gdt_install();
    idt_install();
    pmm_init(128 * 1024 * 1024);
    paging_install();
    heap_init(0x00400000, 16 * 1024 * 1024); // 16 MB heap at 4 MB
    memcpy(&vbe_mode_info, (void*)0x8000, sizeof(VbeModeInfo));

    // Initialize Framebuffer and Console
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
    my_font.bitmap=font; // Use the font array from font.h
    
    console_init(&fb, &my_font);

    console_write("SimpleOS Kernel v1.0 [SECURED]\n");
    console_write("===============================\n\n");
    
    syscalls_install();
    timer_install();
    keyboard_install();
    __asm__ __volatile__("sti");
    rtl8139_init();
    tasking_install();
    mouse_install();
    cursor_init();

    uint32_t free_mem = pmm_get_free_memory();
    console_write("Free memory: ");
    console_write_dec(free_mem / 1024);
    console_write(" KB\n");
    
    console_write("\nKernel initialization complete!\n");
    console_write("Starting GUI...\n\n");

    // --- GUI INITIALIZATION FIXES ---

    // 1. Initialize Managers & Fonts FIRST
    init_widget_system();
    window_manager_init();
    dirty_rect_init();
    widget_set_font(&my_font); // Set font BEFORE creating/drawing widgets

    // 2. Clear Screen
    clear_screen(&fb, 0xECECEC); // Use a nice gray background instead of black

    // 3. Create Window
    Window* main_window = create_window(100, 100, 400, 300, "Welcome to SimpleOS!", true);
    if (!main_window) {
        console_write("Error: Failed to create main window!\n"); // [FIX] Use console_write
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    // 4. Create Widgets
    Widget* label = create_label(20, 40, 200, 30, "Hello, SimpleOS!", 0x000000);
    if (!label) {
        console_write("Error: Failed to create label widget!\n"); // [FIX] Use console_write
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    Widget* button = create_button(100, 100, 100, 50, "Click Me", 0xFFFFFF, 0x0000FF, 0x000000, 0x000000, 1, 0x000000, 0x000000, 0x000000);
    if (!button) {
        console_write("Error: Failed to create button widget!\n"); // [FIX] Use console_write
        for(;;) { __asm__ __volatile__("cli; hlt"); }
    }

    // 5. Assemble UI
    window_add_widget(main_window, label);
    window_add_widget(main_window, button);
    
    button->onClick = on_my_button_click;
    button->onRelease = on_my_button_release;
    
    window_list_head = main_window;
    window_list_tail = main_window;

    // 6. Initial Draw
    window_draw(main_window, &fb);
    dirty_rect_add(0, 0, fb.width, fb.height);

    uint8_t last_buttons = 0;
    int32_t last_x = -1, last_y = -1; 

    // Main Loop
    while(1) {
        bool mouse_moved = (mouse_x != last_x || mouse_y != last_y);
        bool buttons_changed = (mouse_buttons != last_buttons);

        if (mouse_moved || buttons_changed) {
            // Redraw only affected areas
            dirty_rect_add(last_x, last_y, 16, 16); // Old cursor pos
            window_manager_handle_mouse(&window_list_head, &window_list_tail, mouse_x, mouse_y, mouse_buttons, last_buttons);
            dirty_rect_add(mouse_x, mouse_y, 16, 16); // New cursor pos

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
                    // Simple AABB collision check to see if window needs update
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
        __asm__ __volatile__("hlt");
    }
}