//This is for the connection header
#ifndef PCI_H
#define PCI_H
#include <stdint.h>
void pci_scan(void);
uint32_t pci_read_config(uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset);
#endif