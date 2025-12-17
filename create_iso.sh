#!/bin/bash

# Stop on error
set -e

echo "Assembling entry point..."
nasm -f win32 kernel_entry.asm -o kernel_entry.o

echo "Assembling bootloader..."
nasm -f bin boot.asm -o boot.bin

echo "Assembling interrupts..."
nasm -f win32 interrupt.asm -o interrupt.o

echo "Compiling C sources..."
# Note: nic.c is used
C_SOURCES="console.c cursor.c dirty_rect.c font.c gdt.c graphics.c heap.c idt.c kernel.c keyboard.c mouse.c mutex.c paging.c pci.c pmm.c shell.c syscall.c task.c timer.c vga.c widget.c window.c nic.c lib.c"

OBJ_FILES="kernel_entry.o interrupt.o"

for file in $C_SOURCES; do
    obj="${file%.*}.o"
    i686-w64-mingw32-gcc -m32 -ffreestanding -O2 -Wall -Wextra -I. -c "$file" -o "$obj"
    OBJ_FILES="$OBJ_FILES $obj"
done

echo "Linking kernel..."
# Linking kernel_entry.o FIRST is crucial so it ends up at 0x100000
# -Wl,--image-base,0 fixes "section below image base" error on MinGW
i686-w64-mingw32-gcc -Wl,--image-base,0 -Ttext 0x100000 -o kernel.elf -ffreestanding -O2 -nostdlib $OBJ_FILES -lgcc

echo "Extracting binary..."
# Try to find the correct objcopy
if command -v i686-w64-mingw32-objcopy &> /dev/null; then
    i686-w64-mingw32-objcopy -O binary kernel.elf kernel.bin
else
    objcopy -O binary kernel.elf kernel.bin
fi

echo "Creating OS image..."
cat boot.bin kernel.bin > os-image.bin

echo "Creating Floppy Image..."
# Create 1.44MB floppy image
dd if=/dev/zero of=floppy.img bs=1024 count=1440 2>/dev/null
dd if=os-image.bin of=floppy.img conv=notrunc 2>/dev/null

echo "Creating ISO..."
mkdir -p iso_root
cp floppy.img iso_root/

# Generate ISO with floppy emulation
if command -v mkisofs &> /dev/null; then
    mkisofs -quiet -o simpleos.iso -b floppy.img iso_root/
    echo "Build Complete: simpleos.iso"
else
    echo "Warning: 'mkisofs' not found. Skipping ISO creation."
    echo "You can use 'floppy.img' directly in VirtualBox (Floppy Controller) or QEMU."
    echo "Build Complete: floppy.img"
fi