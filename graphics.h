#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
typedef struct __attribute__((packed)){
    uint16_t attributes;
    uint8_t windowA, windowB;
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t windowASegment, windowBSegment;
    uint32_t winFuncPtr;
    uint16_t pitch;
    uint16_t resolution_x, resolution_y;
    uint8_t wChar, yChar, planes, bitsPerPixel,banks;
    uint8_t memoryModel, bankSize, imagePages;
    uint8_t reserved1;
    uint8_t redMask, redMaskPosition;
    uint8_t greenMask, greenMaskPosition;
    uint8_t blueMask, blueMaskPosition;
    uint8_t rsvdMask, rsvdMaskPosition;
    uint8_t directColorAttributes;
    uint32_t physbase; 
    uint32_t reserved2;
    uint16_t reserved3;
} VbeModeInfo;
typedef struct{
    void* address;
    uint32_t        width;
    uint32_t        height;
    uint32_t        pitch;
    uint8_t         bitsPerPixel;
    uint8_t         bytesPerPixel;
} FrameBuffer;
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t  data;
}Bitmap;
typedef struct {
    Bitmap* bitmap;
    uint32_t char_width;
    uint32_t char_height;
}Font;
void clear_screen(FrameBuffer* fb, uint32_t color);
void put_pixel(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t color);
void draw_line(FrameBuffer* fb, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
void draw_rectangle(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void fill_rectangle(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void draw_circle(FrameBuffer* fb, uint32_t x, uint32_t y, uint32_t radius, uint32_t color);
void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, uint32_t x, uint32_t y, uint32_t color);
void draw_char(FrameBuffer* fb, Font* font, char c, uint32_t x, uint32_t y, uint32_t color);
void draw_string(FrameBuffer* fb, Font* font, const char* str, uint32_t x, uint32_t y, uint32_t color);
#endif