#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h"

void syscalls_install(void);
void syscall_handler(registers_t *r);

#endif