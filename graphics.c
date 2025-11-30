#include "graphics.h"
#define FONT_TRANSPARENT_COLOR 0xFFFF00FF  // Magenta as transparent color
void put_pixel(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb->width || y >= fb->height) return; 
    uint8_t* pixel = (uint8_t*)fb->address + y * fb->pitch + x * fb->bytesPerPixel;

    if (fb->bitsPerPixel == 24) {
        pixel[0] = color & 0xFF;         // Blue
        pixel[1] = (color >> 8) & 0xFF;  // Green
        pixel[2] = (color >> 16) & 0xFF; // Red
    } else if (fb->bitsPerPixel == 32) {
        *(uint32_t*)pixel = color;
    }
}
void clear_screen(FrameBuffer* fb, uint32_t color) {
    for (uint32_t y = 0; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            put_pixel(fb, x, y, color);
        }
    }
}
void draw_circle(FrameBuffer* fb, uint32_t xc, uint32_t yc, uint32_t r, uint32_t color) {
    int32_t x = (int32_t)r;
    int32_t y = 0;
    int32_t err = 3 - (2 * (int32_t)r);

    while (x >= y) {
        put_pixel(fb, xc + x, yc + y, color);
        put_pixel(fb, xc + y, yc + x, color);
        put_pixel(fb, xc - y, yc + x, color);
        put_pixel(fb, xc - x, yc + y, color);
        put_pixel(fb, xc - x, yc - y, color);
        put_pixel(fb, xc - y, yc - x, color);
        put_pixel(fb, xc + y, yc - x, color);
        put_pixel(fb, xc + x, yc - y, color);

        if (err > 0) {
            err += 4 * (y - x) + 10;
            x--;
        } else {
            err += 4 * y + 6;
        }
        y++;
    }
}
void draw_line(FrameBuffer* fb, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    int32_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int32_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx - dy;
    while (1) {
        if (x0 >= 0 && (uint32_t)x0 < fb->width && y0 >= 0 && (uint32_t)y0 < fb->height) {
            put_pixel(fb, (uint32_t)x0, (uint32_t)y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
void draw_rectangle(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if (width == 0 || height == 0) return;

    uint32_t x1 = x + width - 1;
    uint32_t y1 = y + height - 1;
    draw_line(fb, x, y, x1, y, color);       // Top
    draw_line(fb, x, y1, x1, y1, color);     // Bottom
    draw_line(fb, x, y, x, y1, color);       // Left
    draw_line(fb, x1, y, x1, y1, color);     // Right
}
void fill_rectangle(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t j = y; j < y + height; j++) {
        for (uint32_t i = x; i < x + width; i++) {
            put_pixel(fb, i, j, color);
        }
    }
}
void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, uint32_t x, uint32_t y, uint32_t color) {
    for (uint32_t j = 0; j < bmp->height; j++) {
        for (uint32_t i = 0; i < bmp->width; i++) {
            uint8_t pixel = *((uint8_t*)bmp->data + j * bmp->width + i);
            if (pixel) { // Assuming non-zero pixel is to be drawn
                put_pixel(fb, x + i, y + j, color);
            }
        }
    }
}
void draw_char(FrameBuffer* fb, Font* font, char c, uint32_t x, uint32_t y, uint32_t color) {
    if (c < 32 || c > 126) return; 
    Bitmap* bmp = &font->bitmap[c - 32];
    for (uint32_t j = 0; j < bmp->height; j++) {
        for (uint32_t i = 0; i < bmp->width; i++) {
            uint8_t pixel = *((uint8_t*)bmp->data + j * bmp->width + i);
            if (pixel && pixel != FONT_TRANSPARENT_COLOR) { // Skip transparent pixels
                put_pixel(fb, x + i, y + j, color);
            }
        }
    }
}
void draw_string(FrameBuffer* fb, Font* font, const char* str, uint32_t x, uint32_t y, uint32_t color) {
    while (*str) {
        draw_char(fb, font, *str, x, y, color);
        x += font->char_width;
        str++;
    }
}
#undef FONT_TRANSPARENT_COLOR