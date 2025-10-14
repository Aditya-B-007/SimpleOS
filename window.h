#ifndef WINDOW_H
#define WINDOW_H
#include "graphics.h"
#include "widget.h"
#include <stdbool.h>
typedef struct{
    int x,y,width,height;
    char** title;
    bool has_title_bar;
    Widget* child_widgets_head;
    Widget* child_widgets_tail;
    struct Window* next;
    struct Window* prev;
    struct Window* parent;
    struct Window* child;
    struct Window* sibling;

}Window;
Window* create_window(int x,int y,int width,int height,char** title,bool has_title_bar);
void window_add_widget(Window* window,Widget* widget);
void window_remove_widget(Window* window,Widget* widget);
void window_draw(Window* window,FrameBuffer* fb);
void window_update(Window* window,FrameBuffer* fb);
void window_on_click(Window* window,int mouse_x,int mouse_y,int button);
void window_on_hover(Window* window,int mouse_x,int mouse_y);
void window_on_release(Window* window,int mouse_x,int mouse_y,int button);
void window_on_move(Window* window,int mouse_x,int mouse_y);
#endif