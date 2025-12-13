#include "nic.h"
#include "idt.h"
#include "pci.h"
#include "vga.h"
#include "pmm.h" // Include our new memory manager

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

// RTL8139 Register Offsets (from the I/O base)
#define REG_CONFIG_1    0x52
#define REG_COMMAND     0x37
#define REG_RX_BUF      0x30
#define REG_IMR         0x3C // Interrupt Mask Register
#define REG_ISR         0x3E // Interrupt Service Register
#define REG_RCR         0x44 // Receive Config Register
#include "io.h" // For inw and outw

uint32_t rtl8139_io_base = 0;
uint8_t* rx_buffer;
void rtl8139_handler(registers_t *r) {
    (void)r;
    vga_print_string("[NIC IRQ]");
    uint16_t status = inw(rtl8139_io_base + REG_ISR);

    if (status & 0x1) { // ROK: Receive OK
        vga_print_string(" Packet Received ");
    }
    outw(rtl8139_io_base + REG_ISR, status);
}

void rtl8139_init(void) {
    vga_print_string("Initializing RTL8139... ");

    // --- Find the card ---
    uint8_t found = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t vendor_device_id = pci_read_config(bus, device, 0, 0);
            if ((vendor_device_id & 0xFFFF) == RTL8139_VENDOR_ID && (vendor_device_id >> 16) == RTL8139_DEVICE_ID) {
                uint32_t bar0 = pci_read_config(bus, device, 0, 0x10);
                rtl8139_io_base = bar0 & 0xFFFC;
                found = 1;
                break;
            }
        }
        if (found) break;
    }

    if (!found) {
        vga_print_string("[NOT FOUND]\n");
        return;
    }
    vga_print_string("[OK]\n");
    outb(rtl8139_io_base + REG_CONFIG_1, 0x00);
    outb(rtl8139_io_base + REG_COMMAND, 0x10);
    while((inb(rtl8139_io_base + REG_COMMAND) & 0x10) != 0) { /* wait */ }
    rx_buffer = (uint8_t*)pmm_alloc_pages(4);
    outl(rtl8139_io_base + REG_RX_BUF, (uint32_t)rx_buffer);
    outw(rtl8139_io_base + REG_IMR, 0x0005);
    outl(rtl8139_io_base + REG_RCR, 0x0F);
    outb(rtl8139_io_base + REG_COMMAND, 0x0C);
    irq_install_handler(11, rtl8139_handler);
}