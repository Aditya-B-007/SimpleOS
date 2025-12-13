#!/bin/bash

# Define the ISO file
ISO="simpleos.iso"
FLOPPY="floppy.img"

# Check if any image file exists before trying to run anything
if [ ! -f "$ISO" ] && [ ! -f "$FLOPPY" ]; then
    echo "Error: Neither $ISO nor $FLOPPY found."
    echo "Please run the build commands or build_iso.sh first."
    exit 1
fi

# Try to find QEMU in PATH or common Windows locations
if command -v qemu-system-i386 &> /dev/null; then
    QEMU="qemu-system-i386"
elif [ -f "/c/Program Files/qemu/qemu-system-i386.exe" ]; then
    QEMU="/c/Program Files/qemu/qemu-system-i386.exe"
elif [ -f "/c/Program Files (x86)/qemu/qemu-system-i386.exe" ]; then
    QEMU="/c/Program Files (x86)/qemu/qemu-system-i386.exe"
elif [ -f "/c/Program Files/qemu/qemu-system-x86_64.exe" ]; then
    # 64-bit QEMU can also run 32-bit OSes
    QEMU="/c/Program Files/qemu/qemu-system-x86_64.exe"
else
    echo "QEMU not found. You can bypass QEMU by using the generated files:"
    [ -f "$ISO" ] && echo " - $ISO (Use with VirtualBox/VMware or burn to CD/USB)"
    [ -f "$FLOPPY" ] && echo " - $FLOPPY (Use with VirtualBox Floppy Controller)"
    exit 0
fi

# Check for ISO or Floppy
if [ -f "$ISO" ]; then
    echo "Booting from CD-ROM ($ISO)..."
    "$QEMU" -cdrom "$ISO"
elif [ -f "$FLOPPY" ]; then
    echo "ISO not found (mkisofs might be missing). Booting from Floppy ($FLOPPY)..."
    "$QEMU" -drive format=raw,file="$FLOPPY",index=0,if=floppy
fi