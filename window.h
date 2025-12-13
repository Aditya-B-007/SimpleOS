#ifndef WINDOW_H
#define WINDOW_H
#include "graphics.h"
#include "widget.h"
#include <stdbool.h>
typedef struct Window {
    int x,y,width,height;
    const char* title; // Use const char* for simple, static titles
    bool has_title_bar;
    Widget* child_widgets_head;
    Widget* child_widgets_tail;
    struct Window* next;
    struct Window* prev;
    bool close_button_hovered;

}Window;
void window_manager_init(void);
Window* create_window(int x,int y,int width,int height,const char* title,bool has_title_bar);
void window_add_widget(Window* window,Widget* widget);
void window_remove_widget(Window* window,Widget* widget);
void window_draw(Window* window,FrameBuffer* fb);
void window_update(Window* window,FrameBuffer* fb);
void window_on_click(Window* window,int mouse_x,int mouse_y,int button);
void window_on_hover(Window* window,int mouse_x,int mouse_y);
void window_on_release(Window* window,int mouse_x,int mouse_y,int button);
void window_on_move(Window* window,int mouse_x,int mouse_y);
void wm_process_mouse(int mouse_x,int mouse_y,uint8_t mouse_buttons,uint8_t last_buttons);
void window_destroy(Window** head, Window** tail, Window* win_to_destroy);
void window_bring_to_front(Window** head, Window** tail, Window* win);
void window_manager_handle_mouse(Window** head, Window** tail, int32_t mouse_x, int32_t mouse_y, uint8_t mouse_buttons, uint8_t last_buttons);
void wm_set_focus(Window* window);
void window_free_widgets(Window* window);
void window_handle_key(char key);
void window_free(Window* window);
#endif