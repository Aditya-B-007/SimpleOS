#include "spinlock.h"
void spinlock_init(spinlock_t* lock) {
    *lock = 0; // Initialize the lock to unlocked state
}
unsigned long spinlock_acquire_irqsave(spinlock_t* lock) {
    unsigned long flags;
    asm volatile(
        "pushf\n"          // Save EFLAGS to stack
        "pop %0\n"        // Pop into flags variable
        "cli\n"           // Disable interrupts
        : "=r"(flags)     // Output operand
        :                 // No input operands
        : "memory"       // Clobber memory to prevent reordering
    );

    while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE)) {
        // Spin-wait (busy wait)
        while (*lock) {
            asm volatile("pause"); // Hint to CPU for better power management
        }
    }
    return flags; 
}
void spinlock_release_irqrestore(spinlock_t* lock, unsigned long flags) {
    __atomic_clear(lock, __ATOMIC_RELEASE); // Release the lock

    asm volatile(
        "push %0\n"      // Push saved flags onto stack
        "popf\n"         // Restore EFLAGS from stack
        :                // No output operands
        : "r"(flags)    // Input operand
        : "memory"      // Clobber memory to prevent reordering
    );
}