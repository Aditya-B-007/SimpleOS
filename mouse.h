#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>
#include "idt.h"
typedef struct {
    uint8_t flags;
    int8_t x_delta;
    int8_t y_delta;

}mouse_packet_t;
//Global mouse states
extern mouse_packet_t mouse_packet;
extern volatile int32_t mouse_x;
extern volatile int32_t mouse_y;
extern volatile uint8_t mouse_buttons; //Bit 0: left, Bit 1 : Right, Bit 2: Middle
void mouse_install(void);
#endif