# SimpleOS - A Lightweight x86 Operating System

SimpleOS is a lightweight, robust operating system for the x86 architecture, written in C and Assembly. It provides a foundational set of OS features, including protected mode, memory segmentation, and a full interrupt handling system, making it ideal for laptops, desktops, legacy system revival, or as a base for custom OS development.

## Architecture

### Boot Process
1. **Real Mode Boot**: 16-bit bootloader loads from sector 0
2. **A20 Line**: Enable extended memory access
3. **Protected Mode**: Switch to 32-bit protected mode
4. **Kernel Entry**: Jump to kernel main function

## Building

### Prerequisites

You need a cross-compilation toolchain for i686-elf:
- `i686-elf-gcc` (C compiler)
- `i686-elf-ld` (Linker) 
- `nasm` (Netwide Assembler)

### Installation on Windows:
1. Install MSYS2
2. Install cross-compilation tools:
```bash
pacman -S mingw-w64-i686-gcc nasm
```

### Compilation
```bash
make all        # Build OS image
make clean      # Remove build files
make qemu       # Run with QEMU (if installed)
make bochs      # Run with Bochs (if installed)
make help       # Show available targets
```

## Running

### QEMU (Recommended)
```bash
qemu-system-i386 -fda build/os.img
```

### Bochs
```bash
bochs -f bochsrc.txt
```

### VirtualBox/VMware
1. Create new VM with floppy disk
2. Mount `build/os.img` as floppy image
3. Boot from floppy

## Technical Details

### Memory Layout
- **0x00007C00**: Bootloader load address
- **0x00100000**: Kernel load address (1MB)
- **0x000B8000**: VGA text buffer
- **0x00090000**: Stack pointer

### Interrupts
- **ISR 0-31**: CPU exceptions (division error, page fault, etc.)
- **IRQ 0-15**: Hardware interrupts (timer, keyboard, etc.)
- **PIC Remapping**: Master PIC (0x20-0x27), Slave PIC (0x28-0x2F)

### Color Codes (VGA)
- 0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta
- 6=Brown, 7=Light Grey, 8=Dark Grey, 9=Light Blue
- 10=Light Green, 11=Light Cyan, 12=Light Red
- 13=Light Magenta, 14=Light Brown, 15=White

## Roadmap

Future development is focused on expanding the kernel's capabilities. Key areas on our roadmap include:

- **Agent Tooling**: Expanding the set of kernel functions (tools) the LLM agent can use.
- **Long-Term Memory**: Implementing a file system to give the agent persistent memory.
- **Multitasking**: Allowing the agent to manage and execute multiple background processes.

### Debugging
- Connect GDB: `gdb -ex "target remote :1234"`
- Enable QEMU monitor: Add `-monitor stdio`

## Contributing

SimpleOS is an open-source product and I welcome contributions from the community. If you are interested in contributing, please feel free to fork the repository, make your changes, and submit a pull request. I am actively looking for improvements in drivers, memory management, and performance optimizations.

## License

This product is released under the Apache 2.0 License. See the LICENSE file for details.

## References

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development resource
- [Intel 80386 Manual](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html)
- [Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/Docs/title.htm)
- [JamesM's Kernel Development Tutorials](https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html)
