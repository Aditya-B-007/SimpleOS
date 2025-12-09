# SimpleOS Flat Makefile

# Tool definitions
CC = i686-w64-mingw32-gcc
LD = i686-w64-mingw32-ld
OBJCOPY = i686-w64-mingw32-objcopy
NASM = nasm

# Flags: 32-bit, no standard libs, include current directory
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -I.

# Auto-detect all .c files in the current folder
C_SOURCES = $(wildcard *.c)
# Create a list of .o files to build
OBJ = $(C_SOURCES:.c=.o)

# Default target
all: os.img

# 1. Create the Disk Image (Boot + Kernel)
os.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os.img

# 2. Compile the Bootloader (Flat Binary)
boot.bin: boot.asm
	$(NASM) -f bin boot.asm -o boot.bin

# 3. Compile the Kernel (Binary extraction)
kernel.bin: kernel.tmp
	$(OBJCOPY) -O binary kernel.tmp kernel.bin

# 4. Link the Kernel (at 1MB)
# Note: interrupt.o must be linked!
kernel.tmp: $(OBJ) interrupt.o
	$(LD) -Ttext 0x100000 -o kernel.tmp $(OBJ) interrupt.o

# 5. Compile Assembly Stub (Interrupts)
interrupt.o: interrupt.asm
	$(NASM) -f win32 interrupt.asm -o interrupt.o

# 6. Compile C Files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f *.o *.bin *.tmp os.img