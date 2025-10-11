#include "mutex.h"
#include "idt.h"
#include "vga.h"
#include "spinlock.h"
extern void schedule(void);//Generic scheduler call
extern task_t* get_current_task(void);//Function to get the current task
void mutex_init(mutex_t* mutex){
    spinlock_init(&mutex->lock);//Initializing the guard
    mutex->locked = false;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
}
void mutex_lock(mutex_t* mutex){
    unsigned long flags=spinlock_acquire_irqsave(&mutex->lock);//Acquire the guard
    task_t* current=get_current_task();//Get the current task

    while (mutex->locked) {
        // The mutex is locked, so we need to wait.
        current->state = TASK_BLOCKED;
        
        // Add current task to the end of the wait queue
        current->next = NULL;
        if (mutex->wait_queue == NULL) {
            mutex->wait_queue = current;
        } else {
            task_t* temp = mutex->wait_queue;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = current;
        }

        // Release the spinlock and schedule another task.
        // This must be atomic.
        schedule_and_release_lock(&mutex->lock, flags);

        // When we wake up, re-acquire the spinlock to re-check the mutex state.
        flags = spinlock_acquire_irqsave(&mutex->lock);
    }

        mutex->locked=true;
        mutex->owner=current;
    spinlock_release_irqrestore(&mutex->lock, flags);
}

void mutex_unlock(mutex_t* mutex){
    unsigned long flags=spinlock_acquire_irqsave(&mutex->lock);
    if (mutex->owner!=get_current_task()){
        // Error: current task does not own the mutex
        spinlock_release_irqrestore(&mutex->lock,flags);
        return;
    }
    task_t* next_task=mutex->wait_queue;//dequeue the next waiting task
    if (next_task!=NULL){
        mutex->wait_queue=next_task->next;
        mutex->locked = true; // The lock remains held, just transferred
        mutex->owner=next_task;
        next_task->state=TASK_READY;
    }
    else{
        mutex->locked=false;
        mutex->owner=NULL;
    }
    spinlock_release_irqrestore(&mutex->lock,flags);
}
