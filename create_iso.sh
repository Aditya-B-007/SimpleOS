#!/bin/bash
set -e

# Assemble
nasm -f win32 kernel_entry.asm -o kernel_entry.o
nasm -f bin boot.asm -o boot.bin
nasm -f win32 interrupt.asm -o interrupt.o

# Compile Single Kernel File
i686-w64-mingw32-gcc -m32 -ffreestanding -O2 -Wall -Wextra -I. -c kernel.c -o kernel.o

# Link (Entry point MUST be first)
i686-w64-mingw32-gcc -Wl,--image-base,0 -Ttext 0x100000 -o kernel.elf -ffreestanding -O2 -nostdlib kernel_entry.o kernel.o interrupt.o -lgcc

# Binary Extraction
i686-w64-mingw32-objcopy -O binary kernel.elf kernel.bin

# Create Images
cat boot.bin kernel.bin > os-image.bin
dd if=/dev/zero of=floppy.img bs=1024 count=1440 2>/dev/null
dd if=os-image.bin of=floppy.img conv=notrunc 2>/dev/null

# Create ISO
mkdir -p iso_root
cp floppy.img iso_root/
mkisofs -quiet -o simpleos.iso -b floppy.img iso_root/

echo "Build Complete: simpleos.iso"