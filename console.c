#include "console.h"
#include "graphics.h"
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
            // Simple scrolling
            if (cursor_y + console_font->char_height > console_fb->height) {
                // This would require a memcpy-like function to move screen contents up.
                // For now, we'll just wrap around.
                cursor_y = 0;
            }
            continue;
        }

        draw_char(console_fb, console_font, c, cursor_x, cursor_y, 0xFFFFFF);

        cursor_x += console_font->char_width;
        if (cursor_x + console_font->char_width > console_fb->width) {
            cursor_x = 0;
            cursor_y += console_font->char_height;
        }
    }
}

void console_write_dec(uint32_t n) {
    if (n == 0) {
        console_write("0");
        return;
    }
    char buf[11];
    char* p = &buf[10];
    *p = '\0';
    do {
        *--p = '0' + (n % 10);
        n /= 10;
    } while (n > 0);
    console_write(p);
}