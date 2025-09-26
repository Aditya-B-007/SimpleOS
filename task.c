#include "task.h"
#include "pmm.h"
#include "vga.h"
#include "timer.h" 
#include <string.h> 
static task_t* current_task;
static task_t* ready_queue;
static int next_pid = 1;
void tasking_install(void) {
    vga_print_string("Initializing multitasking... ");
    current_task = (task_t*)pmm_alloc_page();
    memset(current_task, 0, sizeof(task_t));
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->kernel_stack = NULL; 
    current_task->next = NULL;
    ready_queue = current_task;
    irq_install_handler(0, switch_task);

    vga_print_string("[OK]\n");
}
void create_task(char* name, void (*entry_point)(void)) {
    (void)name; // Name is for debugging, unused for now

    task_t* new_task = (task_t*)pmm_alloc_page();
    memset(new_task, 0, sizeof(task_t));

    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    new_task->kernel_stack = pmm_alloc_page() + PAGE_SIZE;

    // Set up the initial register state
    new_task->regs.eip = (uint32_t)entry_point;
    new_task->regs.esp = (uint32_t)new_task->kernel_stack;
    new_task->regs.eflags = 0x202; // Enable interrupts
    task_t* temp = ready_queue;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = new_task;
}

// The main scheduler function, called by the timer
void switch_task(registers_t* r) {
    if (!current_task) return;

    // Save the state of the current task
    current_task->regs = *r;

    // If there's no other task, just return
    if (!current_task->next) {
        return;
    }

    // Move to the next task in the round-robin queue
    task_t* prev_task = current_task;
    current_task = current_task->next;
    
    // If we've reached the end of the list, loop back to the start
    if (!current_task) {
        current_task = ready_queue;
    }

    current_task->state = TASK_RUNNING;

    // Load the state of the new task
    *r = current_task->regs;
}