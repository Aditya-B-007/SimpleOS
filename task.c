#include "task.h"
#include "pmm.h"
#include "vga.h"
#include "timer.h" 
#include "idt.h"
#include <string.h> 
static task_t* current_task;
static task_t* ready_queue;
static int next_pid = 1;
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
    asm volatile("int $0x20");
}
void create_task(char* name, void (*entry_point)(void)) {
    (void)name; // Name is for debugging, unused for now
    asm volatile("cli");
    task_t* new_task = (task_t*)pmm_alloc_page();
    memset(new_task, 0, sizeof(task_t));

    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    // Allocate one physical page for the kernel stack.
    // pmm_alloc_page() returns the base (lowest address) of the page.
    // Since the x86 stack grows downward, set the initial stack pointer
    // to the *top* of the page by adding PAGE_SIZE.
    // Use explicit cast to avoid pointer arithmetic surprises.
    new_task->kernel_stack = (void*)((uint32_t)pmm_alloc_page() + PAGE_SIZE);
    uint32_t stack_top = (uint32_t)new_task->kernel_stack;
    registers_t* initial_regs = (registers_t*)(stack_top - sizeof(registers_t));
    memset(initial_regs, 0, sizeof(registers_t));
    initial_regs->eax = 0; 
    initial_regs->ebx = 0;
    initial_regs->ecx = 0;
    initial_regs->edx = 0;
    initial_regs->esi = 0;
    initial_regs->edi = 0;
    initial_regs->ebp = 0;
    initial_regs->eip = (uint32_t)entry_point;
    initial_regs->cs = 0x08;
    initial_regs->eflags = 0x202; 
    new_task->regs.esp = (uint32_t)initial_regs;

    // Insert into the circular ready queue
    if (ready_queue == NULL) {
        ready_queue = new_task;
        new_task->next = new_task; // Point to itself
    } else {
        new_task->next = ready_queue->next;
        ready_queue->next = new_task;
    }
    asm volatile("sti");
}

void schedule(registers_t* r) {
    if (!current_task) return;

    // Save the state of the current task
    current_task->regs = *r;

    // If the current task is still running (and not blocked/sleeping), set it to ready
    if (current_task->state == TASK_RUNNING) {
        current_task->state = TASK_READY;
    }

    // Find the next ready task
    task_t* next_task = current_task;
    do {
        next_task = next_task->next;
        if (next_task->state == TASK_READY) {
            break;
        }
    } while (next_task != current_task);

    // If no other task is ready, and current is not ready, we might idle.
    // But if we found a ready task (even if it's the current one), we switch.
    if (next_task->state != TASK_READY) {
        // If current_task is also not ready, we have a problem.
        // For now, we just return, which continues the current context.
        // In a real scenario, you'd switch to an idle task.
        if(current_task->state != TASK_READY) {
            current_task->state = TASK_RUNNING; // Prevent getting stuck
        }
        return;
    }

    // Switch to the next task
    current_task = next_task;
    current_task->state = TASK_RUNNING;

    // Restore the state of the new current task
    *r = current_task->regs;
}

void schedule_and_release_lock(spinlock_t* lock, unsigned long flags) {
    spinlock_release_irqrestore(lock, flags);
    schedule_from_yield();
}