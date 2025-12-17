#include "task.h"
#include "pmm.h"
#include "vga.h"
#include "timer.h" 
#include "idt.h"
#include <string.h>
static task_t* current_task;
static task_t* ready_queue;
static int next_pid = 1;

void idle_task_func(void){
    while(1){
        __asm__ __volatile__("hlt");
    }
}
task_t* get_current_task(void) {
    return current_task;
}
void tasking_install(void) {
    vga_print_string("Initializing multitasking... ");
    current_task = (task_t*)pmm_alloc_page();
    memset(current_task, 0, sizeof(task_t));
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->kernel_stack = NULL; 
    current_task->next = NULL;
    ready_queue = current_task; // This will be our idle task

    vga_print_string("[OK]\n");
}
void schedule_from_yield(void){
    __asm__ __volatile__("int $0x20");
}
void create_task(char* name, void (*entry_point)(void)) {
    (void)name; // Name is for debugging, unused for now
    __asm__ __volatile__("cli");
    task_t* new_task = (task_t*)pmm_alloc_page();
    memset(new_task, 0, sizeof(task_t));

    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    new_task->kernel_stack = (void*)((uint32_t)pmm_alloc_page() + PAGE_SIZE);
    uint32_t stack_top = (uint32_t)new_task->kernel_stack;
    registers_t* initial_regs = (registers_t*)(stack_top - sizeof(registers_t));
    memset(initial_regs, 0, sizeof(registers_t));
    initial_regs->eip = (uint32_t)entry_point;
    initial_regs->eflags = 0x202; // Enable interrupts
    new_task->regs.esp = (uint32_t)initial_regs;
    initial_regs->cs = 0x08; // Kernel code segment
    initial_regs->ds = 0x10; // Kernel data segment
    // Add to the end of the ready queue
    task_t* temp = ready_queue;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_task;
    new_task->next = ready_queue; 
    __asm__ __volatile__("sti");
}



void schedule(registers_t* r) {
    if (!current_task) return;

    // Save the state of the curreint task
    current_task->regs = *r;
    task_t* next_task = current_task->next;
    while (next_task && next_task->state != TASK_READY) {
        next_task = next_task->next;
        if (next_task == current_task) {
            // No other ready tasks found
            next_task = current_task;
            break;
        }
    }
    current_task = next_task;
    current_task->state = TASK_RUNNING;
    *r = current_task->regs;
}

void schedule_and_release_lock(spinlock_t* lock, unsigned long flags) {
    spinlock_release_irqrestore(lock, flags);
    schedule_from_yield();
}