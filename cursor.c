#include "cursor.h"
#include "dirty_rect.h"
#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
static const uint8_t cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH / 8] = {
    {0b00000000, 0b00000000},
    {0b00000000, 0b10000000},
    {0b00000001, 0b11000000},
    {0b00000011, 0b11100000},
    {0b00000111, 0b11110000},
    {0b00001111, 0b11111000},
    {0b00011111, 0b11111100},
    {0b00111111, 0b11111110},
    {0b01111111, 0b11111111},
    {0b00111111, 0b11111110},
    {0b00011111, 0b11111100},
    {0b00001111, 0b11111000},
    {0b00000111, 0b11110000},
    {0b00000011, 0b11100000},
    {0b00000001, 0b11000000},
    {0b00000000, 0b10000000}
};
static uint32_t cursor_bg_buffer[CURSOR_HEIGHT*CURSOR_WIDTH];
static uint32_t cursor_fg_buffer[CURSOR_HEIGHT*CURSOR_WIDTH];
static int prev_x = -1, prev_y = -1;
static int visible = 0;
void cursor_init(void) {
    // Initialization if needed
    prev_x = -1;
    prev_y = -1;
    visible = 0;
}
void cursor_draw(FrameBuffer* fb, int x, int y) {
    for (int cy = 0; cy < CURSOR_HEIGHT; cy++) {
        for (int cx = 0; cx < CURSOR_WIDTH; cx++) {
            int byte_index = cx / 8;
            int bit_index = 7 - (cx % 8);
            uint8_t mask = 1 << bit_index;
            int fb_x = x + cx;
            int fb_y = y + cy;
            if (fb_x < 0 || fb_x >=(int)fb->width || fb_y < 0 || fb_y >= (int)fb->height) continue;
            uint32_t* fb_pixel = (uint32_t*)((uint8_t*)fb->address + (fb_y * fb->pitch) + (fb_x * (fb->bitsPerPixel / 8)));
            if (cursor_bitmap[cy][byte_index] & mask) {
                cursor_fg_buffer[cy * CURSOR_WIDTH + cx] = *fb_pixel;
                *fb_pixel = 0xFFFFFFFF; // White color for cursor
            } else {
                cursor_bg_buffer[cy * CURSOR_WIDTH + cx] = *fb_pixel;
                *fb_pixel = 0x00000000; // Black color for cursor background
            }
        }
    }
    visible = 1;
    prev_x = x;
    prev_y = y;
}
void cursor_hide(void) {
    if (!visible) return;
    visible = 0;
}
void cursor_update(FrameBuffer* fb, int x, int y) {
    if (visible) {
        // Invalidate the area where the cursor was
        dirty_rect_add(prev_x, prev_y, CURSOR_WIDTH, CURSOR_HEIGHT);

        for (int cy = 0; cy < CURSOR_HEIGHT; cy++) {
            for (int cx = 0; cx < CURSOR_WIDTH; cx++) {
                int fb_x = prev_x + cx;
                int fb_y = prev_y + cy;
                if (fb_x < 0 || fb_x >= (int)fb->width || fb_y < 0 || fb_y >= (int)fb->height) continue;
                uint32_t* fb_pixel = (uint32_t*)((uint8_t*)fb->address + (fb_y * fb->pitch) + (fb_x * (fb->bitsPerPixel / 8)));
                if (cursor_bitmap[cy][cx / 8] & (1 << (7 - (cx % 8)))) {
                    *fb_pixel = cursor_fg_buffer[cy * CURSOR_WIDTH + cx];
                } else {
                    *fb_pixel = cursor_bg_buffer[cy * CURSOR_WIDTH + cx];
                }
            }
        }
    }
    // Invalidate the area where the cursor will be
    dirty_rect_add(x, y, CURSOR_WIDTH, CURSOR_HEIGHT);

    cursor_draw(fb, x, y);
}
#undef CURSOR_WIDTH
#undef CURSOR_HEIGHT
#undef cursor_bitmap
#undef CURSOR_BG_BUFFER
#undef CURSOR_FG_BUFFER
#undef CURSOR_VISIBLE