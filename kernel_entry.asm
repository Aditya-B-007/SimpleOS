bits 32

section .text
; The default entry point for the MinGW GCC toolchain is _start.
global _start
extern kernel_main

_start:
    ; The bootloader has already set up the GDT and entered protected mode.
    ; We just need to establish a stack and call our C kernel.
    mov esp, kernel_stack_top ; Set the stack pointer.
    call kernel_main          ; Call the C kernel_main() function.
    cli                       ; If kernel_main returns, halt the CPU.
.hang:
    hlt
    jmp .hang

section .bss
resb 8192 ; Reserve 8KB for the kernel stack.
kernel_stack_top:

