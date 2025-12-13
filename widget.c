#include "widget.h"
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "shell.h"
#include "pci.h"
Font* g_widget_font=NULL;
void widget_set_font(Font* font){
    g_widget_font=font;
}
void draw_label(Widget* self,FrameBuffer* fb){
    if(!g_widget_font)return;
    LabelData* data=(LabelData*)self->data;
    if(!data||!data->text)return;
    draw_string(fb, g_widget_font, data->text, self->x, self->y, data->color);
}
void update_label(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
}
void draw_button(Widget* self,FrameBuffer* fb){
    if(!g_widget_font)return;
    ButtonData* data=(ButtonData*)self->data;
    if(!data||!data->text)return;
    // Draw button background
    for(int y=0;y<self->height;y++){
        for(int x=0;x<self->width;x++){
            uint32_t color=data->bg_color;
            if(x<self->width/2){
                if(y<self->height/2){
                    color=data->base_color;
                }else{
                    color=data->hover_color;
                }
            }else{
                if(y<self->height/2){
                    color=data->hover_color;
                }else{
                    color=data->base_color;
                }
            }
            put_pixel(fb,self->x+x,self->y+y,color);
        }
    }
    // Draw button border
    for(int i=0;i<data->border_width;i++){
        for(int y=i;y<self->height-i;y++){
            for(int x=i;x<self->width-i;x++){
                uint32_t color=data->border_color;
                if(x==i||x==self->width-i-1||y==i||y==self->height-i-1){
                    put_pixel(fb,self->x+x,self->y+y,color);
                }
            }
        }
    }
    // Draw button text (centered)
    int text_width=strlen(data->text)*g_widget_font->char_width;
    int text_height=g_widget_font->char_height;
    int text_x=self->x+self->width/2-text_width/2;
    int text_y=self->y+self->height/2-text_height/2;
    draw_string(fb, g_widget_font, data->text, text_x, text_y, data->text_color);
}
void update_button(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
}
void draw_textbox(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
    if(!g_widget_font)return;
}
void update_textbox(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
}
void draw_scrollbar(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
}
void update_scrollbar(Widget* self,FrameBuffer* fb){
    (void)self; (void)fb;
}
void handle_button_click(Widget* self,int mouse_x,int mouse_y,int button){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    // Change button color on click
    if(button==1){ // Left click
        data->bg_color=data->press_color;
        data->border_color=data->press_border;
    }
}
void handle_button_release(Widget* self,int mouse_x,int mouse_y,int button){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    if(button==1){ // Left click
        data->bg_color=data->base_color;
        data->border_color=data->base_color;
    }
}
void handle_button_hover(Widget* self,int mouse_x,int mouse_y){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    // Change button color on hover
    data->bg_color=data->hover_color;
    data->border_color=data->hover_border;
}
void handle_button_leave(Widget* self,int mouse_x,int mouse_y){
    (void)mouse_x; (void)mouse_y;
    ButtonData* data=(ButtonData*)self->data;
    if(!data)return;
    // Revert button color when not hovering
    data->bg_color=data->base_color;
    data->border_color=data->base_color;
}

typedef struct {
    char* placeholder;
    char* text;
    uint32_t bg_color;
    uint32_t text_color;
} TextboxData;
void widget_draw(Widget* widget,FrameBuffer* fb){
    if(widget&&widget->draw){
        widget->draw(widget,fb);
    }
}
void widget_update(Widget* widget,FrameBuffer* fb){
    if(widget&&widget->update){
        widget->update(widget,fb);
    }
}
void widget_handle_event(Widget* widget,int mouse_x,int mouse_y,int event){
    if (!widget) return;
    if (mouse_x >= widget->x && mouse_x < widget->x + widget->width &&
        mouse_y >= widget->y && mouse_y < widget->y + widget->height) {
        
        if (event == EVENT_MOUSE_CLICK && widget->onClick) {
            widget->onClick(widget, mouse_x, mouse_y, 1); // Assuming left button for now
        } else if (event == EVENT_MOUSE_RELEASE && widget->onRelease) {
            widget->onRelease(widget, mouse_x, mouse_y, 1); // Assuming left button for now
        } else if (event == EVENT_MOUSE_HOVER && widget->onHover) {
            widget->onHover(widget, mouse_x, mouse_y);
        } else if (event == EVENT_MOUSE_MOVE && widget->onMove) {
            widget->onMove(widget, mouse_x, mouse_y);
        }
    }
}
void widget_add(Widget** head,Widget* new_widget){
    if(!head||!new_widget)return;
    new_widget->next=*head;
    *head=new_widget;
}
void widget_remove(Widget** head,Widget* widget){
    if(!head||!widget)return;
    if(*head==widget){
        *head=widget->next;
        return;
    }
    Widget* current=*head;
    while(current&&current->next!=widget){
        current=current->next;
    }
    if(current){
        current->next=widget->next;
    }
}
void widget_free(Widget* widget){
    if(!widget)return;
    if(widget->data)free(widget->data);
    free(widget);
}
void widget_free_all(Widget** head){
    if(!head)return;
    Widget* current=*head;
    while(current){
        Widget* next=current->next;
        widget_free(current);
        current=next;
    }
    *head=NULL;
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
Widget* create_label(int x,int y,int width,int height,char* text,uint32_t color){
    LabelData* data=(LabelData*)malloc(sizeof(LabelData));
    if(!data)return NULL;
    data->text=text;
    data->color=color;
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){
        free(data);
        return NULL;
    }
    widget->x=x;
    widget->y=y;
    widget->width=width;
    widget->height=height;
    widget->data=data;
    widget->draw=draw_label;
    widget->update=update_label;
 return widget;
}
Widget* create_button(int x,int y,int width,int height,char* text,uint32_t base_color,uint32_t hover_color,uint32_t press_color,uint32_t border_color,int border_width,uint32_t text_color,uint32_t press_border,uint32_t hover_border){
    ButtonData* data=(ButtonData*)malloc(sizeof(ButtonData));
    if(!data)return NULL;
    data->text=text;
    data->base_color=base_color;
    data->hover_color=hover_color;
    data->press_color=press_color;
    data->border_color=border_color;
    data->border_width=border_width;
    data->text_color=text_color;
    data->press_border=press_border;
    data->hover_border=hover_border;
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){
        free(data);
        return NULL;
    }
    widget->x=x;
    widget->y=y;
    widget->width=width;
    widget->height=height;
    widget->data=data;
    widget->draw=draw_button;
    widget->update=update_button;
    widget->onClick=handle_button_click;
    widget->onRelease=handle_button_release;
    widget->onHover=handle_button_hover;
    widget->onMove=NULL;
 return widget;
}
Widget* create_textbox(int x,int y,int width,int height,char* placeholder,uint32_t bg_color,uint32_t text_color){
    TextboxData* data=(TextboxData*)malloc(sizeof(TextboxData));
    if(!data)return NULL;
    data->placeholder=placeholder;
    data->bg_color=bg_color;
    data->text_color=text_color;
    data->text=(char*)malloc(256); // Fixed size for simplicity
    if(!data->text){
        free(data);
        return NULL;
    }
    data->text[0]='\0';
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){
        free(data->text);
        free(data);
        return NULL;
    }
    widget->x=x;
    widget->y=y;
    widget->width=width;
    widget->height=height;
    widget->data=data;
    widget->draw=draw_textbox;
    widget->update=update_textbox;
    return widget;
}

typedef struct {
    uint32_t bg_color;
    uint32_t thumb_color;
    int thumb_pos;
    int thumb_size;
} ScrollbarData;
Widget* create_scrollbar(int x,int y,int width,int height,uint32_t bg_color,uint32_t thumb_color){
    ScrollbarData* data=(ScrollbarData*)malloc(sizeof(ScrollbarData));
    if(!data)return NULL;
    data->bg_color=bg_color;
    data->thumb_color=thumb_color;
    data->thumb_pos=0;
    data->thumb_size=height/4; // Example size
    Widget* widget=(Widget*)malloc(sizeof(Widget));
    if(!widget){
        free(data);
        return NULL;
    }
    widget->x=x;
    widget->y=y;
    widget->width=width;
    widget->height=height;
    widget->data=data;
    widget->draw=draw_scrollbar;
    widget->update=update_scrollbar;
    return widget;
}
void init_widget_system(){
    // Initialize default font
    // g_widget_font=load_font("default_font_path"); // Placeholder path

    // if(!g_widget_font){
    // Fallback to built-in font
    // g_widget_font=get_builtin_font();
    // }
    if(!g_widget_font){
        // If still no font, log error
        vga_print_string("Failed to load widget font\n");
    }
    // Initialize other widget system components if needed
}
    