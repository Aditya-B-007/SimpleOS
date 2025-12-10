#include "gdt.h"
#include <stdint.h>

#define GDT_ENTRIES 5

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}
void gdt_install(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                    // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    // Kernel code segment (Ring 0)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    // Kernel data segment (Ring 0)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    // User code segment (Ring 3, limited)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    // User data segment (Ring 3, limited)

    gdt_flush();
}

extern void gdt_flush_asm(void);

void gdt_flush(void) {
    asm volatile (
        "lgdt %0\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "ljmp $0x08, $1f\n\t"
        "1:\n\t"
        :
        : "m"(gp)
        : "eax"
    );
}