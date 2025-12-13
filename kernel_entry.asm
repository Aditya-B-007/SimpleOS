[bits 32]
[extern _kernel_main]
[global start]

section .text
start:
    call _kernel_main
    jmp $