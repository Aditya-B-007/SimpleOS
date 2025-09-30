#include "timer.h"
#include "task.h" 
volatile uint32_t ticks = 0;
void timer_handler(registers_t *r) {
    ticks++;
    schedule(r);
}
uint32_t get_ticks() {
    return ticks;
}
void timer_install() {
    int frequency = 100;
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);
    irq_install_handler(0, timer_handler); 
}