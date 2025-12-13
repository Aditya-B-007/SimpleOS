#!/bin/bash

# Check if the raw OS image exists
if [ ! -f os-image.bin ]; then
    echo "Error: os-image.bin not found. Run the build commands first."
    exit 1
fi

# 1. Create a blank 1.44MB floppy image (1474560 bytes)
dd if=/dev/zero of=floppy.img bs=1024 count=1440

# 2. Write the OS image (bootloader + kernel) to the floppy image
#    conv=notrunc ensures we overwrite the start without truncating the file
dd if=os-image.bin of=floppy.img conv=notrunc

# 3. Create the ISO directory structure
mkdir -p iso_root
cp floppy.img iso_root/

# 4. Generate the ISO using mkisofs (Floppy Emulation Mode)
#    -b specifies the boot image. Since it is 1.44MB, mkisofs enables floppy emulation.
mkisofs -o simpleos.iso -b floppy.img iso_root/

echo "Done! You can now boot simpleos.iso in a VM."