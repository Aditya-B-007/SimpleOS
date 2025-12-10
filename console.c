#include "console.h"
static FrameBuffer* console_fb = NULL;
static Font* console_font = NULL;
void console_init(FrameBuffer* fb, Font* font) {
    console_fb = fb;
    console_font = font;
}
void console_write(const char* str) {
    if (!console_fb || !console_font) return;
    static uint32_t cursor_x = 0;
    static uint32_t cursor_y = 0;

    while (*str) {
        char c = *str++;
        if (c == '\n') {
            cursor_x = 0;
            cursor_y += console_font->char_height;
            continue;
        }

        cursor_x += console_font->char_width;
        if (cursor_x + console_font->char_width > console_fb->width) {
            cursor_x = 0;
            cursor_y += console_font->char_height;
        }
    }
}
#undef console_fb
#undef console_font
#undef console_write
#define console_init console_init
#define console_write console_write
#define console_fb console_fb
#define console_font console_font
#define console_write console_write
#define console_init console_init
#define console_fb console_fb
#define console_font console_font
#define console_write console_write
#define console_init console_init
#define console_fb console_fb