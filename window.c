#include "window.h"
#include <stdlib.h>
#include <string.h>
#define WINDOW_BG_COLOR   0xECECEC   
#define TITLE_BAR_COLOR   0x2C2C2C   
#define TITLE_TEXT_COLOR  0xFFFFFF   
#define TITLE_BAR_HEIGHT  25 
#define CLOSE_BUTTON_WIDTH  18
#define CLOSE_BUTTON_HEIGHT 18
#define CLOSE_BUTTON_MARGIN 4
#define CLOSE_BUTTON_BG_COLOR 0xFF0000 // Red
#define CLOSE_BUTTON_HOVER_BG_COLOR 0xFF4444 // Brighter Red for hover
#define CLOSE_BUTTON_X_COLOR  0xFFFFFF // White
extern Font* g_widget_font;
extern Window* window_list_head;
extern Window* window_list_tail;
static Widget* last_hivered_widget = NULL;
Window* create_window(int x,int y,int width,int height,char** title,bool has_title_bar){
    Window* window=(Window*)malloc(sizeof(Window));
    if(!window)return NULL;
    window->x=x;
    window->y=y;
    window->width=width;
    window->height=height;
    window->title=title;
    window->has_title_bar=has_title_bar;
    window->child_widgets_head=NULL;
    window->child_widgets_tail=NULL;
    window->next=NULL;
    window->prev=NULL;
    window->parent=NULL;
    window->child=NULL;
    window->sibling=NULL;
    window->close_button_hovered = false;
    return window;
}
void window_add_widget(Window* window,Widget* widget){
    if(!window||!widget)return;
    if(!window->child_widgets_head){
        window->child_widgets_head=widget;
        window->child_widgets_tail=widget;
    }else{
        window->child_widgets_tail->next=widget;
        window->child_widgets_tail=widget;
    }
    widget->next=NULL; // Ensure the new widget's next is NULL
}
void window_remove_widget(Window* window,Widget* widget){
    if(!window||!widget||!window->child_widgets_head)return;
    if(window->child_widgets_head==widget){
        window->child_widgets_head=widget->next;
        if(window->child_widgets_tail==widget){
            window->child_widgets_tail=NULL;
        }
        return;
    }
    Widget* current=window->child_widgets_head;
    while(current&&current->next!=widget){
        current=current->next;
    }
    if(current){
        current->next=widget->next;
        if(window->child_widgets_tail==widget){
            window->child_widgets_tail=current;
        }
    }
}
void window_free_widgets(Window* window){
    if(!window)return;
    Widget* current=window->child_widgets_head;
    while(current){
        Widget* next=current->next;
        if(current->data)free(current->data);
        free(current);
        current=next;
    }
    window->child_widgets_head=NULL;
    window->child_widgets_tail=NULL;
}
void window_free(Window* window){
    if(!window)return;
    window_free_widgets(window);
    free(window);
}
void window_draw(Window* window,FrameBuffer* fb){
    if(!window)return;
    // Draw window background
    for(int y=0;y<window->height;y++){
        for(int x=0;x<window->width;x++){
            fb_set_pixel(fb,window->x+x,window->y+y,WINDOW_BG_COLOR);
        }
    }
    // Draw title bar if present
    if(window->has_title_bar&&window->title){
        for(int y=0;y<TITLE_BAR_HEIGHT;y++){
            for(int x=0;x<window->width;x++){
                fb_set_pixel(fb,window->x+x,window->y+y,TITLE_BAR_COLOR);
            }
        }
        // Draw title text (centered)
        if(g_widget_font){
            int text_width=strlen(*window->title)*g_widget_font->char_width;
            int text_height=g_widget_font->char_height;
            int text_x=window->x+window->width/2-text_width/2;
            int text_y=window->y+(TITLE_BAR_HEIGHT-text_height)/2;
            draw_string(fb,text_x,text_y,*window->title,TITLE_TEXT_COLOR,g_widget_font);
        }

        // Draw close button
        int btn_x = window->x + window->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
        int btn_y = window->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;
        uint32_t btn_color = window->close_button_hovered ? CLOSE_BUTTON_HOVER_BG_COLOR : CLOSE_BUTTON_BG_COLOR;
        fill_rectangle(fb, btn_x, btn_y, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, btn_color);

        // Draw 'X' on the button
        int x_margin = 4;
        draw_line(fb, btn_x + x_margin, btn_y + x_margin, 
                      btn_x + CLOSE_BUTTON_WIDTH - x_margin - 1, btn_y + CLOSE_BUTTON_HEIGHT - x_margin - 1, 
                      CLOSE_BUTTON_X_COLOR);
        draw_line(fb, btn_x + x_margin, btn_y + CLOSE_BUTTON_HEIGHT - x_margin - 1,
                      btn_x + CLOSE_BUTTON_WIDTH - x_margin - 1, btn_y + x_margin,
                      CLOSE_BUTTON_X_COLOR);
    }
    // Draw child widgets
    widget_draw_all(window->child_widgets_head,fb);
}
void window_update(Window* window,FrameBuffer* fb){
    if(!window)return;
    widget_update_all(window->child_widgets_head,fb);
}
void window_handle_event(Window* window,int mouse_x,int mouse_y,int event){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,event);
}
void window_on_click(Window* window,int mouse_x,int mouse_y,int button){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,1); // 1 for click
}
void window_on_release(Window* window,int mouse_x,int mouse_y,int button){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,2); // 2 for release
}
void window_on_hover(Window* window,int mouse_x,int mouse_y){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,3); // 3 for hover
}
void window_on_move(Window* window,int mouse_x,int mouse_y){
    if(!window)return;
    widget_handle_event_all(window->child_widgets_head,mouse_x,mouse_y,4); // 4 for move
}
void widget_draw_all(Widget* head,FrameBuffer* fb){
    Widget* current=head;
    while(current){
        widget_draw(current,fb);
        current=current->next;
    }
}

void window_destroy(Window** head, Window** tail, Window* win_to_destroy) {
    if (!win_to_destroy) return;

    // Unlink from the list
    if (win_to_destroy->prev) {
        win_to_destroy->prev->next = win_to_destroy->next;
    } else { // It's the head
        *head = win_to_destroy->next;
    }

    if (win_to_destroy->next) {
        win_to_destroy->next->prev = win_to_destroy->prev;
    } else { // It's the tail
        *tail = win_to_destroy->prev;
    }

    // Free associated widgets and the window itself
    window_free_widgets(win_to_destroy);
    free(win_to_destroy);
}


void window_bring_to_front(Window** head, Window** tail, Window* win) {
    if (!win || *tail == win) {
        return; // Already at the front or invalid window
    }

    // Unlink from current position
    if (win->prev) {
        win->prev->next = win->next;
    } else { // It's the head
        *head = win->next;
    }

    if (win->next) {
        win->next->prev = win->prev;
    } else { // It's the tail - this case should not happen if *tail != win
        *tail = win->prev;
    }

    // Add to the tail (front of the drawing order)
    (*tail)->next = win;
    win->prev = *tail;
    win->next = NULL;
    *tail = win;
}

// State for window dragging, kept static within this file
static Window* dragged_window = NULL;
static int32_t drag_offset_x = 0;
static int32_t drag_offset_y = 0;

void window_manager_handle_mouse(Window** head, Window** tail, int32_t mouse_x, int32_t mouse_y, uint8_t mouse_buttons, uint8_t last_buttons) {

    Window* top_win = *tail;
    if (top_win) {
        int btn_x = top_win->x + top_win->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
        int btn_y = top_win->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;

        if (mouse_x >= btn_x && mouse_x < btn_x + CLOSE_BUTTON_WIDTH &&
            mouse_y >= btn_y && mouse_y < btn_y + CLOSE_BUTTON_HEIGHT) {
            top_win->close_button_hovered = true;
        } else {
            top_win->close_button_hovered = false;
        }
    }


    // 1. Mouse button pressed
    if ((mouse_buttons & 1) && !(last_buttons & 1)) {
        // Iterate from top window (tail) to bottom to find which one was clicked
        Window* win = *tail;
        if (win) { // Only proceed if there is at least one window
            // Check for close button click first
            int btn_x = win->x + win->width - CLOSE_BUTTON_WIDTH - CLOSE_BUTTON_MARGIN;
            int btn_y = win->y + (TITLE_BAR_HEIGHT - CLOSE_BUTTON_HEIGHT) / 2;
            if (mouse_x >= btn_x && mouse_x < btn_x + CLOSE_BUTTON_WIDTH &&
                mouse_y >= btn_y && mouse_y < btn_y + CLOSE_BUTTON_HEIGHT) {
                
                window_destroy(head, tail, win);
                // Don't proceed to drag checks

            } else if (mouse_x >= win->x && mouse_x < win->x + win->width &&
                       mouse_y >= win->y && mouse_y < win->y + TITLE_BAR_HEIGHT) {
                // It's a drag operation
                dragged_window = win;
                drag_offset_x = mouse_x - win->x;
                drag_offset_y = mouse_y - win->y;

                window_bring_to_front(head, tail, win);
            } else {
                // Click was outside the top window's title bar, maybe handle widget clicks here in the future
            }
        }
    }

    // 2. Mouse button released
    if (!(mouse_buttons & 1) && (last_buttons & 1)) {
        dragged_window = NULL;
    }

    // 3. Mouse moved while dragging
    if (dragged_window != NULL) {
        dragged_window->x = mouse_x - drag_offset_x;
        dragged_window->y = mouse_y - drag_offset_y;
    }
}
void widget_draw(Widget* widget,FrameBuffer* fb){
    if(!widget||!widget->draw)return;
    widget->draw(widget,fb);
}
void widget_update(Widget* widget,FrameBuffer* fb){
    if(!widget||!widget->update)return;
    widget->update(widget,fb);
}
void widget_handle_event(Widget* widget,int mouse_x,int mouse_y,int event){
    if(!widget||!widget->handle_event)return;
    widget->handle_event(widget,mouse_x,mouse_y,event);
}
void widget_handle_event_all(Widget* head,int mouse_x,int mouse_y,int event){
    Widget* current=head;
    while(current){
        widget_handle_event(current,mouse_x,mouse_y,event);
        current=current->next;
    }
}
void widget_draw_all(Widget* head,FrameBuffer* fb){
    Widget* current=head;
    while(current){
        widget_draw(current,fb);
        current=current->next;
    }
}
void widget_update_all(Widget* head,FrameBuffer* fb){
    Widget* current=head;
    while(current){
        widget_update(current,fb);
        current=current->next;
    }
}
void widget_handle_event_all(Widget* head,int mouse_x,int mouse_y,int event){
    Widget* current=head;
    while(current){
        widget_handle_event(current,mouse_x,mouse_y,event);
        current=current->next;
    }
}
void wm_process_mouse(int mouse_x,int mouse_y,uint8_t mouse_buttons,uint8_t last_buttons){
    window_manager_handle_mouse(&window_list_head,&window_list_tail,mouse_x,mouse_y,mouse_buttons,last_buttons);
}
