#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include "idt.h" 
typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_SLEEPING,
    TASK_DEAD
} task_state_t;

// This is the Task Control Block (TCB)
typedef struct task {
    int id;                 // Process ID
    registers_t regs;       // Saved CPU state
    void* kernel_stack;     // Pointer to the top of the kernel stack
    task_state_t state;     // Current state of the task
    struct task* next;      // Next task in the linked list
} task_t;
void tasking_install(void);
void create_task(char* name, void (*entry_point)(void));
void switch_task(registers_t* r);

#endif