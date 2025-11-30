bits 16
org 0x7c00
VBE_MAGIC equ 0x4F
VBE_GET_INFO equ 0x00
VBE_GET_MODE_INFO equ 0x01
VBE_SET_MODE equ 0x02
TARGET_WIDTH equ 1280
TARGET_HEIGHT equ 720
TARGET_BPP equ 32
;extern kernel_main

start:
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    in al, 0x92
    or al, 2
    out 0x92, al
    mov ax, VBE_MAGIC + VBE_GET_INFO
    mov di, vbe_info_block
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error
    mov cx, [vbe_info_block + 14]
    mov si, [vbe_info_block + 16]
.mode_loop:
    push cx
    mov ax, VBE_MAGIC + VBE_GET_MODE_INFO
    mov cx, [si]
    mov di, mode_info_block
    int 0x10
    cmp ax, 0x004F
    jne .next_mode
    cmp word [mode_info_block +18], TARGET_WIDTH
    jne .next_mode
    cmp word [mode_info_block +20], TARGET_HEIGHT
    jne .next_mode
    cmp word [mode_info_block +25], TARGET_BPP
    jne .next_mode
    mov ax, VBE_MAGIC + VBE_SET_MODE
    mov bx, [si]
    or bx, 1 << 14 ;For the Linear Frame Buffer (LFB)
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error
    mov edi, 0x8000
    mov esi, mode_info_block 
    mov ecx, 256
    rep movsb
    jmp .continue_boot
.next_mode:
    add si, 2
    pop cx 
    loop .mode_loop
.graphics_error:
    cli
    hlt
.continue_boot
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:start_protected_mode

bits 32
start_protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000  ; Set stack pointer to safe location
    push dword 0x8000
    call kernel_main
    cli
    hlt
    jmp $
; GDT for bootloader
gdt_start:
    dd 0x0, 0x0
    dw 0xFFFF, 0x0000, 0x9A, 0xCF, 0x00
    dw 0xFFFF, 0x0000, 0x92, 0xCF, 0x00
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; Limit
    dd gdt_start
bss_start:
    vbe_info_block resb 512
    mode_info_block resb 256
bss_end:
; Boot signature
times 510-($-$$) db 0
dw 0xAA55