#include "mouse.h"
#include "vga.h"
#include "idt.h" 
#include "io.h"
#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64
#define MOUSE_IRQ 12
volatile int32_t mouse_x = 400;
volatile int32_t mouse_y = 300;
volatile uint8_t mouse_buttons = 0;
uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];
mouse_packet_t mouse_packet;
static void mouse_handler(registers_t *regs);

void mouse_install(void) {
    // Enable the auxiliary mouse device
    outb(MOUSE_COMMAND_PORT, 0xA8);
    // Enable the mouse IRQ
    outb(MOUSE_COMMAND_PORT, 0x20);
    uint8_t status = inb(MOUSE_DATA_PORT);
    status |= 2; // Enable IRQ12
    outb(MOUSE_COMMAND_PORT, 0x60);
    outb(MOUSE_DATA_PORT, status);
    // Tell the mouse to use default settings
    outb(MOUSE_COMMAND_PORT, 0xD4);
    outb(MOUSE_DATA_PORT, 0xF6);
    inb(MOUSE_DATA_PORT); // Acknowledge
    // Enable the mouse
    outb(MOUSE_COMMAND_PORT, 0xD4);
    outb(MOUSE_DATA_PORT, 0xF4);
    inb(MOUSE_DATA_PORT); // Acknowledge
    // Register the mouse handler
    irq_install_handler(MOUSE_IRQ, mouse_handler);
}
static void mouse_handler(registers_t *regs) {
    (void)regs;
    static uint8_t packet_byte_index = 0;
    static mouse_packet_t packet;
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if (status & 0x20) {
        uint8_t data = inb(MOUSE_DATA_PORT);
        switch (packet_byte_index) {
            case 0:
                packet.flags = data;
                break;
            case 1:
                packet.x_delta = (int8_t)data;
                break;
            case 2:
                packet.y_delta = (int8_t)data;
                break;
        }
        packet_byte_index++;
        if (packet_byte_index == 3) {
            mouse_packet = packet;
            mouse_x += packet.x_delta;
            mouse_y -= packet.y_delta; // Invert Y axis
            mouse_buttons = packet.flags & 0x07; // Update button states
            packet_byte_index = 0; // Reset for next packet
        }
    }
}