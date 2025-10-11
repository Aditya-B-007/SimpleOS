#ifndef MUTEX_H
#define MUTEX_H
#include "task.h"
#include "spinlock.h"
#include <stdbool.h>
typedef struct mutex {
    spinlock_t lock; // Spinlock in order to ensure that the atomic operations on the mutex itself are thread-safe
    bool locked;// This is for checking if the mutex is currrently being held or not
    task_t* owner;//The task that currently holds the lock
    task_t* wait_queue;//A linked list of tasks waiting for the lock.
} mutex_t;
//Function prototypes for the mutexes operations we would be defining
void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);
#endif//MUTEX_H