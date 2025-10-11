#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <stdbool.h>
#include <stdint.h>
typedef volatile int spinlock_t;
void spinlock_init(spinlock_t* lock);
unsigned long spinlock_acquire_irqsave(spinlock_t* lock);
void spinlock_release_irqrestore(spinlock_t* lock, unsigned long flags);
#endif