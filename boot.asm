bits 16
org 0x7c00

; --- Memory Definitions (NOT File Space) ---
VBE_INFO_BLOCK  equ 0x9000
MODE_INFO_BLOCK equ 0x9200

; --- Constants ---
VBE_MAGIC equ 0x4F
VBE_GET_INFO equ 0x00
VBE_GET_MODE_INFO equ 0x01
VBE_SET_MODE equ 0x02
TARGET_WIDTH equ 1280
TARGET_HEIGHT equ 720
TARGET_BPP equ 32

; Kernel placement
KERNEL_TEMP_ADDR equ 0x1000 ; Segment 0x1000 (Physical 0x10000)
KERNEL_TARGET_ADDR equ 0x100000 ; 1MB
SECTORS_TO_READ  equ 60     ; Read 30KB

start:
    jmp 0:init_segments     ; Far jump to normalize CS

init_segments:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    
    mov [boot_drive], dl    ; Save the boot drive number provided by BIOS

    ; 1. Load Kernel from Disk to Temp Buffer
    call load_kernel

    ; 2. Enable A20 Line
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 3. VBE Graphics Setup
    mov ax, VBE_MAGIC + VBE_GET_INFO
    mov di, VBE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error
    
    mov cx, [VBE_INFO_BLOCK + 14]
    mov si, [VBE_INFO_BLOCK + 16]

.mode_loop:
    push cx
    mov ax, VBE_MAGIC + VBE_GET_MODE_INFO
    mov cx, [si]
    mov di, MODE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne .next_mode
    
    cmp word [MODE_INFO_BLOCK + 18], TARGET_WIDTH
    jne .next_mode
    cmp word [MODE_INFO_BLOCK + 20], TARGET_HEIGHT
    jne .next_mode
    cmp word [MODE_INFO_BLOCK + 25], TARGET_BPP
    jne .next_mode
    
    ; Found mode! Set it.
    mov ax, VBE_MAGIC + VBE_SET_MODE
    mov bx, [si]
    or bx, 1 << 14
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error
    
    ; Save VBE info for kernel
    push es
    xor ax, ax
    mov es, ax
    mov edi, 0x8000
    mov esi, MODE_INFO_BLOCK 
    mov ecx, 256
    rep movsb
    pop es
    
    ; FIX: Jump to the global label (no dot)
    jmp enter_pm

.next_mode:
    add si, 2
    pop cx 
    loop .mode_loop

.graphics_error:
    cli
    hlt

; --- Disk Loading Routine ---
load_kernel:
    mov ax, KERNEL_TEMP_ADDR
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02
    mov al, SECTORS_TO_READ 
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, [boot_drive]    
    int 0x13
    jc .disk_error
    ret

.disk_error:
    cli
    hlt

; --- Protected Mode Entry (Global Label) ---
enter_pm:
    cli
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
    mov esp, 0x90000
    
    ; Copy Kernel to 1MB
    mov esi, 0x10000
    mov edi, KERNEL_TARGET_ADDR
    mov ecx, (SECTORS_TO_READ * 512) / 4 
    rep movsd

    ; Jump to Kernel (1MB)
    push dword 0x8000
    mov eax, KERNEL_TARGET_ADDR
    call eax
    
    cli
    hlt

; Variables
boot_drive db 0
gdt_start:
    dd 0x0, 0x0
    dw 0xFFFF, 0x0000, 0x9A, 0xCF, 0x00
    dw 0xFFFF, 0x0000, 0x92, 0xCF, 0x00
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55