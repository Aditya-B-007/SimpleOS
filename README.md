# SimpleOS - A Lightweight x86 Operating System

SimpleOS is a lightweight, robust operating system for the x86 architecture, written in C and Assembly. It provides a foundational set of OS features, including protected mode, memory segmentation, and a full interrupt handling system, making it ideal for embedded applications, legacy system revival, or as a base for custom OS development.

## Features

- **Bootloader**: Custom 16-bit bootloader that transitions to protected mode
- **GDT (Global Descriptor Table)**: Memory segmentation with kernel/user segments
- **IDT (Interrupt Descriptor Table)**: Complete interrupt handling system with exception messages
- **VGA Text Driver**: Full-featured text output with scrolling, colors, and formatting
- **Keyboard Driver**: PS/2 keyboard support with scan code translation
- **Interactive Shell**: Command-line interface with built-in commands
- **Memory Management**: Basic protected mode memory setup
- **Interrupt Handling**: Proper CPU exception and hardware interrupt handling

## Architecture

### Boot Process
1. **Real Mode Boot**: 16-bit bootloader loads from sector 0
2. **A20 Line**: Enable extended memory access
3. **Protected Mode**: Switch to 32-bit protected mode
4. **Kernel Entry**: Jump to kernel main function

### Kernel Components

#### VGA Driver (`vga.c`, `vga.h`)
- Text mode output at 0xB8000
- 80x25 character display
- 16-color palette support
- Automatic scrolling
- Hex/decimal number formatting

#### GDT Manager (`gdt.c`, `gdt.h`)  
- 5-entry descriptor table (null, kernel code/data, user code/data)
- Proper segment reloading
- 4GB flat memory model

#### IDT Manager (`idt.c`, `idt.h`)
- 256 interrupt descriptors
- Exception handlers with descriptive messages
- Hardware interrupt support (PIC reprogramming)
- Assembly interrupt stubs (`interrupt.asm`)

#### Keyboard Driver (`keyboard.c`, `keyboard.h`)
- PS/2 keyboard interrupt handler (IRQ1)
- Scan code to ASCII translation
- Shift and Caps Lock support
- Key buffer for input queuing
- Integration with shell interface

#### Interactive Shell (`shell.c`, `shell.h`)
- Command-line interface
- Built-in commands: help, clear, about, reboot, halt
- Command history and input processing
- Backspace support and line editing

#### Kernel Main (`kernel.c`)
- System initialization sequence  
- Colorized boot messages
- Shell initialization and main loop

## Building

### Prerequisites

You need a cross-compilation toolchain for i686-elf:
- `i686-elf-gcc` (C compiler)
- `i686-elf-ld` (Linker) 
- `nasm` (Netwide Assembler)

### Installation on Ubuntu/Debian:
```bash
sudo apt-get install build-essential nasm qemu-system-x86
# For the cross-compiler, you can build it from source.
# A good guide can be found here: https://wiki.osdev.org/GCC_Cross-Compiler
# Alternatively, some package managers might have pre-built toolchains.
```

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

## Using the Shell

Once the system boots, you'll see the SimpleOS shell prompt:

```
SimpleOS> 
```

The agent will process your request and respond accordingly.

### Example Interactions
- `> what time is it?`
- `> create a new file named notes.txt`
- `> tell me about the CPU`
- `> reboot the machine in 5 minutes`

## File Structure

```
SimpleOS/
├── boot/
│   └── boot.asm          # 16-bit bootloader
├── kernel/
│   ├── kernel.c          # Kernel main and initialization
│   ├── gdt.c/.h          # Global Descriptor Table
│   ├── idt.c/.h          # Interrupt Descriptor Table  
│   ├── vga.c/.h          # VGA text driver
│   ├── keyboard.c/.h     # PS/2 keyboard driver
│   ├── agent.c/.h        # The core AI agent logic
│   └── interrupt.asm     # Assembly interrupt handlers
├── build/                # Build output (auto-generated)
├── Makefile             # Build configuration
├── linker.ld            # Linker script
└── README.md            # This file
```

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

SimpleOS is an open-source product and we welcome contributions from the community. If you are interested in contributing, please feel free to fork the repository, make your changes, and submit a pull request. We are actively looking for improvements in drivers, memory management, and performance optimizations.

## License

This product is released under the Apache 2.0 License. See the LICENSE file for details.

## References

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development resource
- [Intel 80386 Manual](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html)
- [Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/Docs/title.htm)
- [JamesM's Kernel Development Tutorials](https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html)
