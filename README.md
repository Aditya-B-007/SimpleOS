*An Operating System that is light on the system and employs ML based algorithms for its kernel. It is an Agentic AI based OS*
*OS STILL UNDER DEVELOPMENT*

System Calls: Interface needs to be robust. This is the primary mechanism for a user program to request services (like writing to the screen or reading from a file) from the kernel. Need to define a clear set of syscalls and a stable way to invoke them (usually via int 0x80).

Userspace and ELF Loader: It involves: Writing a simple "hello world" program in C and compiling it to an ELF (Executable and Linkable Format) binary. Writing an ELF parser in your kernel that can read this binary. Creating a new virtual address space (page directory and tables) for the user process. Loading the program's code and data into that address space. Finally, performing the transition from kernel mode (Ring 0) to user mode (Ring 3) to start the program.

Virtual Filesystem (VFS) and Initial RAM Disk (initrd): To load the user program from, you'll need a basic filesystem. The easiest way to start is with an initrd, where you load your user programs into memory with the kernel at boot time, and the kernel reads them from that memory region as if it were a disk.

Advanced Drivers: With a stable core, we can now more safely revisit complex drivers like the RTL8139 NIC, keeping the DMA and physical memory requirements in mind.
