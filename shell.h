#ifndef SHELL_H
#define SHELL_H

#include "vga.h"
#include <stdint.h>

void shell_init(void);
void shell_handle_input(char ch);
void shell_print_prompt(void);

#endif