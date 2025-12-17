#ifndef CURSOR_H
#define CURSOR_H
#include <stdint.h>
#include "graphics.h"
void cursor_init(void);
void cursor_update(FrameBuffer*fb,int x,int y);
void cursor_draw(FrameBuffer*fb,int x,int y);
#endif // CURSOR_H