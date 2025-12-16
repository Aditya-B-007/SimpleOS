CC = gcc
LD = ld
OBJCOPY = objcopy
NASM = nasm

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -I. -std=gnu99
# Use -f win32 for MinGW, -f elf32 for Linux/Cross-Compiler
NASMFLAGS = -f win32
LDFLAGS = -T linker.ld

# Auto-detect all .c files in the current folder
C_SOURCES = $(wildcard *.c)
# Exclude spinlock.c if it exists (replaced by sync.h implementation)
C_SOURCES := $(filter-out spinlock.c, $(C_SOURCES))
# Create a list of .o files to build
OBJ = $(C_SOURCES:.c=.o)

# Default target
all: os.img

os.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os.img

boot.bin: boot.asm
	$(NASM) -f bin boot.asm -o boot.bin

kernel.bin: kernel.tmp
	$(OBJCOPY) -O binary kernel.tmp kernel.bin

kernel.tmp: $(OBJ) interrupt.o
	$(LD) $(LDFLAGS) -o kernel.tmp $(OBJ) interrupt.o

interrupt.o: interrupt.asm
	$(NASM) $(NASMFLAGS) interrupt.asm -o interrupt.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.bin *.tmp os.img