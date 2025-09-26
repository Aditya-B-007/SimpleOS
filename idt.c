#include "idt.h"
#include "vga.h"

static struct idt_entry idt[256];
static struct idt_ptr ip;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}
void isr_handler(registers_t *r) {
    // Validate register structure pointer
    if (r == NULL) {
        vga_print_string("Critical error: NULL register pointer in ISR!\n");
        asm volatile("cli");
        for(;;) { asm volatile("hlt"); }
    }

    vga_print_string("Received interrupt: ");
    vga_print_dec(r->int_no);
    vga_print_string(" - ");

    if (r->int_no < 32) {
        // Handle CPU exceptions
        if (r->int_no < sizeof(exception_messages) / sizeof(exception_messages[0])) {
            vga_print_string(exception_messages[r->int_no]);
        } else {
            vga_print_string("Unknown Exception");
        }
        vga_print_string("\nEIP: 0x");
        vga_print_hex(r->eip);
        vga_print_string(" CS: 0x");
        vga_print_hex(r->cs);
        vga_print_string(" EFLAGS: 0x");
        vga_print_hex(r->eflags);
        vga_print_string(" ESP: 0x");
        vga_print_hex(r->esp);

        // Additional debug information for certain exceptions
        if (r->int_no == 13) { // General Protection Fault
            vga_print_string("\nError code: 0x");
            vga_print_hex(r->err_code);
        }

        vga_print_string("\nSystem halted due to exception!\n");
        asm volatile("cli");
        for(;;) {
            asm volatile("hlt");
        }
    }
}

static void (*irq_routines[16])(registers_t *r);

void irq_install_handler(int irq, void (*handler)(registers_t *r)) {
    if (irq >= 0 && irq < 16 && handler != NULL) {
        irq_routines[irq] = handler;
    }
}

void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = 0;
    }
}

void irq_handler(registers_t *r) {
    // Validate register structure pointer
    if (r == NULL) {
        return;
    }

    // Validate interrupt number range for IRQs
    if (r->int_no < 32 || r->int_no > 47) {
        return;
    }

    int irq = r->int_no - 32;

    // Double-check IRQ bounds
    if (irq >= 0 && irq < 16) {
        void (*handler)(registers_t *r) = irq_routines[irq];
        if (handler != NULL) {
            // Call the registered handler
            handler(r);
        }
    }

    // Send End of Interrupt signal to PICs
    if (r->int_no >= 40) {
        // Send reset signal to slave PIC
        outb(0xA0, 0x20);
    }
    // Send reset signal to master PIC
    outb(0x20, 0x20);
}

void idt_install(void) {
    ip.limit = sizeof(struct idt_entry) * 256 - 1;
    ip.base = (uint32_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)page_fault_handler, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    asm volatile("lidt %0" : : "m"(ip));
    asm volatile("sti");
}
uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}
void page_fault_handler(registers_t *r) {
    // Function is called when there is a page fault
    // The faulting address is stored in the CR2 register
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us the details about the fault
    int present = !(r->err_code & 0x1);   // Page is not present
    int rw = r->err_code & 0x2;           // Write operation?
    int us = r->err_code & 0x4;           // Were we in user-mode?
    int reserved = r->err_code & 0x8;     // Overwritten reserved bits?
    int id = (r->err_code & 0x10) >> 4;   // Instruction fetch (if NX enabled)
    int pk = (r->err_code & 0x20) >> 5;   // Protection key violation (if PKU enabled)
    int ss = (r->err_code & 0x40) >> 6;   // Shadow stack violation (Intel CET)
    int sgx = (r->err_code & 0x80) >> 7;  // SGX access violation (if SGX enabled)

    // Display detailed page fault information
    vga_print_string("\n\nPAGE FAULT!\n");
    vga_print_string("Faulting address: 0x");
    vga_print_hex(faulting_address);
    vga_print_string("\nError code: 0x");
    vga_print_hex(r->err_code);
    vga_print_string("\nEIP: 0x");
    vga_print_hex(r->eip);

    vga_print_string("\nFault type: ");
    if (!present) vga_print_string("Page not present ");
    if (rw) vga_print_string("Write operation ");
    if (us) vga_print_string("User mode ");
    if (reserved) vga_print_string("Reserved bits overwritten ");
    if (id) vga_print_string("Instruction fetch ");
    if (pk) vga_print_string("Protection key violation ");
    if (ss) vga_print_string("Shadow stack violation ");
    if (sgx) vga_print_string("SGX access violation ");

    vga_print_string("\nSystem halted due to page fault!\n");

    // Prevent unused variable warnings
    (void)id; (void)pk; (void)ss; (void)sgx;

    asm volatile("cli");
    for(;;) {
        asm volatile("hlt");
    }
}