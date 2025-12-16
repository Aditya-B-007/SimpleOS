#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
typedef struct {
    uint16_t attributes;
    uint8_t  windowA, windowB;
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t windowASegment, windowBSegment;
    uint32_t winFuncPtr;
    uint16_t pitch;
    uint16_t resolution_x, resolution_y;
    uint8_t  wChar, yChar, planes, bitsPerPixel, banks;
    uint8_t  memoryModel, bankSize, imagePages;
    uint8_t  reserved1;
    uint8_t  redMask, redMaskPosition;
    uint8_t  greenMask, greenMaskPosition;
    uint8_t  blueMask, blueMaskPosition;
    uint8_t  rsvdMask, rsvdMaskPosition;
    uint8_t  directColorAttributes;
    uint32_t physbase;
    uint32_t reserved2;
    uint16_t reserved3;
} VbeModeInfo;

extern VbeModeInfo vbe_mode_info;
typedef struct{
    void* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bitsPerPixel;
    uint8_t bytesPerPixel;
} FrameBuffer;
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* data; 
} Bitmap;
typedef struct {
    uint32_t char_width;
    uint32_t char_height;
    const uint8_t (*bitmap)[16]; 
} Font;
void clear_screen(FrameBuffer* fb, uint32_t color);
void put_pixel(FrameBuffer* fb, int32_t x, int32_t y, uint32_t color);
void draw_line(FrameBuffer* fb, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void draw_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void fill_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void draw_circle(FrameBuffer* fb, int32_t x, int32_t y, int32_t radius, uint32_t color);
void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, int32_t x, int32_t y, uint32_t color);
void draw_char(FrameBuffer* fb, Font* font, char c, int32_t x, int32_t y, uint32_t color);
void draw_string(FrameBuffer* fb, Font* font, const char* str, int32_t x, int32_t y, uint32_t color);

#endif