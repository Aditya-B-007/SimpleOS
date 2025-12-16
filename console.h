#ifndef CONSOLE_H
#define CONSOLE_H
#include <graphics.h>
#include <stdint.h>
#include <stddef.h>
void console_init(FrameBuffer* fb, Font* font);
void console_write(const char* str);
void console_write_dec(uint32_t n);
#endif // CONSOLE_H