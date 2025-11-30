#include "syscall.h"
#include "vga.h"
void syscall_handler(registers_t *r){
	if (r->eax==0){
		vga_print_string("SYSTEM CALL RECEIVED!, SIMPLE OS WORKING\n");
	}
}