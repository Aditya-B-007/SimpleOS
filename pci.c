#include "pci.h"
#include "idt.h" 
#include "vga.h"
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunc << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_scan(void) {
    vga_print_string("Scanning PCI bus...\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t vendor_device_id = pci_read_config(bus, device, 0, 0);
            if ((vendor_device_id & 0xFFFF) != 0xFFFF) {
                uint16_t vendor = vendor_device_id & 0xFFFF;
                uint16_t device_id = vendor_device_id >> 16;
                vga_print_string("Found PCI device - Vendor: ");
                vga_print_hex(vendor);
                vga_print_string(", Device: ");
                vga_print_hex(device_id);
                vga_print_string("\n");
            }
        }
    }
}