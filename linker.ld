/* The entry point is the `_start` symbol from kernel_entry.asm */
ENTRY(_start)

SECTIONS
{
    /* The kernel will be loaded at 1MB. */
    . = 1M;

    .text :
    {
        *(.text)   /* All executable code. */
    }

    .rodata :
    {
        *(.rodata*) /* All read-only data. */
    }

    .data :
    {
        *(.data)   /* All initialized data. */
    }

    .bss :
    {
        *(.bss)    /* All uninitialized data. */
    }
}

