bits 16
org 0x7c00

; --- Memory Definitions ---
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
KERNEL_TEMP_ADDR equ 0x1000 
KERNEL_TARGET_ADDR equ 0x100000 
SECTORS_TO_READ  equ 100

start:
    jmp 0:init_segments

init_segments:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    
    mov [boot_drive], dl    ; Save boot drive

    ; 1. Load Kernel (Fixed Routine)
    call load_kernel_safe
    
    ; CRITICAL FIX: Ensure ES is 0 for VBE calls
    xor ax, ax
    mov es, ax

    ; 2. Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 3. VBE Graphics Setup
    mov ax, VBE_MAGIC + VBE_GET_INFO
    mov di, VBE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error
    
    ; Get video modes list pointer
    mov cx, [VBE_INFO_BLOCK + 14] ; Offset
    mov si, [VBE_INFO_BLOCK + 16] ; Segment
    mov fs, si                    ; Use FS to access the mode list segment

    ; Note: The list is a far pointer (Seg:Off). 
    ; We need to read from FS:CX (Seg:Off).
    ; But for simplicity in Real Mode flat memory (if Seg is not 0), 
    ; we usually normalize.
    ; If BIOS returns a segment for the list, we must use it.
    ; Simplification: Many BIOSes return Segment in SI.
    
    mov si, cx ; Move Offset to SI
    mov ax, [VBE_INFO_BLOCK + 16] 
    mov ds, ax ; Point DS to the VBE Mode List Segment

.mode_loop:
    mov cx, [si]          ; Read mode number
    cmp cx, 0xFFFF        ; End of list?
    je .graphics_error
    
    push cx
    
    ; Restore DS to 0 for the Interrupt buffer access
    push ds 
    xor ax, ax
    mov ds, ax
    
    mov ax, VBE_MAGIC + VBE_GET_MODE_INFO
    mov di, MODE_INFO_BLOCK
    int 0x10
    
    cmp ax, 0x004F
    pop ds ; Restore Mode List Segment
    jne .next_mode_pop

    ; Check resolution (Need to use ES or explicit segment 0 override since DS is changed)
    ; But wait, MODE_INFO_BLOCK is at 0x9200. We need to check it using DS=0.
    ; It's cleaner to just switch DS back to 0 for the check.
    push ds
    xor ax, ax
    mov ds, ax
    
    cmp word [MODE_INFO_BLOCK + 18], TARGET_WIDTH
    jne .check_failed
    cmp word [MODE_INFO_BLOCK + 20], TARGET_HEIGHT
    jne .check_failed
    cmp word [MODE_INFO_BLOCK + 25], TARGET_BPP
    jne .check_failed
    
    ; FOUND IT!
    pop ds ; Restore stack balance (pushed DS)
    pop cx ; Restore Mode Number (pushed CX)
    jmp .set_mode

.check_failed:
    pop ds
.next_mode_pop:
    pop cx 
    add si, 2
    jmp .mode_loop

.set_mode:
    ; CX holds the mode number
    mov bx, cx
    or bx, 1 << 14 ; Linear Frame Buffer bit
    
    ; Reset DS to 0 before final call
    xor ax, ax
    mov ds, ax
    
    mov ax, VBE_MAGIC + VBE_SET_MODE
    int 0x10
    cmp ax, 0x004F
    jne .graphics_error

    ; Save VBE info for kernel
    ; Copy from 0x9200 (MODE_INFO_BLOCK) to 0x8000
    xor ax, ax
    mov es, ax
    mov ds, ax
    mov edi, 0x8000
    mov esi, MODE_INFO_BLOCK 
    mov ecx, 64 ; Copy 256 bytes (64 dwords)
    rep movsd   ; Use movsd for speed (32-bit width in 16-bit mode needs 0x66 prefix usually, but `rep movsd` works in real mode on 386+)
    
    jmp enter_pm

.graphics_error:
    cli
    hlt

; --- Safe Disk Loading (CHS Loop) ---
load_kernel_safe:
    mov ax, KERNEL_TEMP_ADDR
    mov es, ax      ; Buffer segment
    xor bx, bx      ; Buffer offset 0
    
    mov dl, [boot_drive]
    mov ch, 0       ; Cylinder 0
    mov dh, 0       ; Head 0
    mov cl, 2       ; Start at Sector 2
    
    mov si, SECTORS_TO_READ 

.read_loop:
    mov ah, 0x02
    mov al, 1       ; Read 1 sector at a time
    int 0x13
    jc .disk_error
    
    dec si
    jz .done
    
    add bx, 512     ; Move buffer pointer
    ; Handle Segment overflow (64KB) if necessary, but 50KB fits in one segment.
    
    ; Update CHS
    inc cl          ; Next sector
    cmp cl, 19      ; 18 sectors per track?
    jl .read_loop
    
    ; Next Track/Head logic
    mov cl, 1       ; Reset sector
    inc dh          ; Next head
    cmp dh, 2       ; 2 heads?
    jl .read_loop
    
    mov dh, 0       ; Reset head
    inc ch          ; Next cylinder
    jmp .read_loop

.done:
    ret

.disk_error:
    cli
    hlt

; --- Protected Mode Entry ---
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

    ; Jump to Kernel
    ; Ensure kernel_entry.o is linked FIRST
    jmp KERNEL_TARGET_ADDR
    
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