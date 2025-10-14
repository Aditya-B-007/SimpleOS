#include "window.h"
#include <stdlib.h>
#include <string.h>
#define WINDOW_BG_COLOR   0xECECEC   
#define TITLE_BAR_COLOR   0x2C2C2C   
#define TITLE_TEXT_COLOR  0xFFFFFF   
#define TITLE_BAR_HEIGHT  25
extern Font* g_widget_font;
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

