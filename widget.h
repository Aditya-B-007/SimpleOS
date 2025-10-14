#ifndef WIDGET_H
#define WIDGET_H
#include "graphics.h"
#include <stdbool.h>
struct Widget;
typedef enum {
    WIDGET_LABEL,
    WIDGET_BUTTON,
    WIDGET_CHECKBOX,
    WIDGET_RADIOBUTTON,
    WIDGET_SLIDER,
    WIDGET_SCROLLBAR,
    WIDGET_LISTBOX,
    WIDGET_PANEL,
    WIDGET_TEXTBOX,
    WIDGET_TEXTAREA,
    WIDGET_COMBOBOX,
    WIDGET_MENU,
    WIDGET_TABLE,
    WIDGET_TREE,
    WIDGET_GRAPH,
    WIDGET_DIALOG,
    WIDGET_SPINNER,
    WIDGET_CANVAS,
    WIDGET_PROGRESSBAR,
    WIDGET_FILECHOOSER,
    WIDGET_COLORCHOOSER,
    WIDGET_FONTCHOOSER,
    WIDGET_SLIDER,
    WIDGET_MENU,
    WIDGET_TOGGLEBUTTON,
    WIDGET_RADIOGROUP,
    WIDGET_VOICEINPUT
}WidgetType;
typedef struct Widget{
    WidgetType type;
    int x,y,width,height;
    void (*draw)(struct Widget* self,FrameBuffer* fb);
    void (*update)(struct Widget* self,FrameBuffer* fb);
    void (*onClick)(struct Widget* self,int mouse_x,int mouse_y,int button);
    void (*onHover)(struct Widget* self,int mouse_x,int mouse_y);
    void (*onRelease)(struct Widget* self,int mouse_x,int mouse_y,int button);
    void (*onMove)(struct Widget* self,int mouse_x,int mouse_y);
    void* data;
    struct Widget* next;

}Widget;
typedef struct{
    char* text;
    uint32_t color;
} LabelData;
typedef struct{
    char* text;
    uint32_t color;
    uint32_t bg_color;
    uint32_t border_color;
    int border_width;
    uint32_t base_color;
    int base_width;
    uint32_t text_color;
    uint32_t press_color;
    uint32_t press_border;
    uint32_t hover_color;
    uint32_t hover_border;
} ButtonData;
//Below are the constructors.
Widget* create_label(int x,int y,int width,int height,char* text,uint32_t color);
Widget* create_button(int x,int y,int width,int height,char* text,uint32_t color,uint32_t bg_color,uint32_t border_color,int border_width,uint32_t base_color,int base_width,uint32_t text_color,uint32_t press_color,uint32_t press_border,uint32_t hover_color,uint32_t hover_border);
Widget* create_checkbox(int x,int y,int width,int height,char* text,uint32_t color,uint32_t bg_color,uint32_t border_color,int border_width);

#endif