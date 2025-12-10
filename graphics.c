#include "graphics.h"
void put_pixel(FrameBuffer* fb, int32_t x, int32_t y, uint32_t color) {
    if (x < 0 || x >= (int32_t)fb->width || y < 0 || y >= (int32_t)fb->height) return; 
    
    uint8_t* pixel = (uint8_t*)fb->address + y * fb->pitch + x * fb->bytesPerPixel;

    if (fb->bitsPerPixel == 24) {
        pixel[0] = color & 0xFF;         
        pixel[1] = (color >> 8) & 0xFF;  
        pixel[2] = (color >> 16) & 0xFF; 
    } else if (fb->bitsPerPixel == 32) {
        *(uint32_t*)pixel = color;
    }
}

void clear_screen(FrameBuffer* fb, uint32_t color) {
    // Optimization: We could use memset/memcpy here for speed
    for (uint32_t y = 0; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            put_pixel(fb, x, y, color);
        }
    }
}

void draw_circle(FrameBuffer* fb, int32_t xc, int32_t yc, int32_t r, uint32_t color) {
    int32_t x = r;
    int32_t y = 0;
    int32_t err = 3 - (2 * r);

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
        put_pixel(fb, x0, y0, color);
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

void draw_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    if (width <= 0 || height <= 0) return;
    int32_t x1 = x + width - 1;
    int32_t y1 = y + height - 1;
    draw_line(fb, x, y, x1, y, color);       
    draw_line(fb, x, y1, x1, y1, color);     
    draw_line(fb, x, y, x, y1, color);       
    draw_line(fb, x1, y, x1, y1, color);     
}

void fill_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    for (int32_t j = y; j < y + height; j++) {
        for (int32_t i = x; i < x + width; i++) {
            put_pixel(fb, i, j, color);
        }
    }
}

void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, int32_t x, int32_t y, uint32_t color) {
    if (!bmp || !bmp->data) return;
    for (uint32_t j = 0; j < bmp->height; j++) {
        for (uint32_t i = 0; i < bmp->width; i++) {
            // [FIX] bmp->data is now a pointer, so we access it as an array
            uint8_t pixel = bmp->data[j * bmp->width + i];
            if (pixel) { 
                put_pixel(fb, x + i, y + j, color);
            }
        }
    }
}

// [FIX] Completely rewritten to match the font.c raw array format
void draw_char(FrameBuffer* fb, Font* font, char c, int32_t x, int32_t y, uint32_t color) {
    if (!font || !font->bitmap) return;
    
    // Get the pointer to the 16 bytes for this character
    const uint8_t* glyph = font->bitmap[(unsigned char)c];
    
    for (uint32_t cy = 0; cy < font->char_height; cy++) {
        uint8_t row = glyph[cy];
        for (uint32_t cx = 0; cx < font->char_width; cx++) {
            // Check the bit at this position (Bit 7 is leftmost)
            if ((row >> (7 - cx)) & 1) {
                put_pixel(fb, x + cx, y + cy, color);
            }
        }
    }
}

void draw_string(FrameBuffer* fb, Font* font, const char* str, int32_t x, int32_t y, uint32_t color) {
    while (*str) {
        draw_char(fb, font, *str, x, y, color);
        x += font->char_width;
        str++;
    }
}