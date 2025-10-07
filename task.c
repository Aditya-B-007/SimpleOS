#include "task.h"
#include "pmm.h"
#include "vga.h"
#include "timer.h" 
#include "idt.h"
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

    vga_print_string("[OK]\n");
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
    new_task->kernel_stack = (uint32_t)pmm_alloc_page() + PAGE_SIZE;
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
    new_task->next = NULL;
    task_t* temp = ready_queue;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = new_task;
    asm volatile("sti");
}

void schedule(registers_t* r) {
    if (!current_task) return;
    current_task->regs = *r;
    if (!current_task->next) {
        return;
    }
    task_t* prev_task = current_task;
    current_task = current_task->next;
    if (!current_task) {
        current_task = ready_queue;
    }
    current_task->state = TASK_RUNNING;
    *r = current_task->regs;
}