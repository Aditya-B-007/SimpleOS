#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

typedef struct {
    volatile bool locked;
} spinlock_t;

/**
 * @brief Initializes a spinlock to the unlocked state.
 * @param lock Pointer to the spinlock
 */
static inline void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

/**
 * @brief Acquires the spinlock, spinning until it is free.
 * This uses an atomic test-and-set operation.
 * @param lock Pointer to the spinlock.
 */ 
static inline void spinlock_acquire(spinlock_t* lock) {
    while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE));
}

/**
 * @brief Releases the spinlock.
 * @param lock Pointer to the spinlock.
 */
static inline void spinlock_release(spinlock_t* lock) {
    __atomic_clear(&lock->locked, __ATOMIC_RELEASE);
}

#endif // SYNC_H