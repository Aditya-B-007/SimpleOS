#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include "idt.h" 
typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_SLEEPING,
    TASK_DEAD,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_KILLED,
    TASK_ZOMBIE
} task_state_t;
typedef struct task {
    int id;                 
    registers_t regs;       
    void* kernel_stack;    
    task_state_t state;     
    struct task* next;       
} task_t;
void tasking_install(void);
void create_task(char* name, void (*entry_point)(void));
void schedule(registers_t* r);
void schedule_and_release_lock(spinlock_t* lock, unsigned long flags);

#endif