#include <stdint.h>
#include "paging.h"
#include "vga.h"
//Static allocation currently, 4MB=4096-byte aligned
__attribute__((aligned(4096))) pt_entry_t page_directory[1024];
__attribute__((aligned(4096))) pt_entry_t fpage_table[1024];

void paging_install(void) {
    vga_print_string("Installing paging... ");

    // Verify page directory and table are properly aligned (4KB boundary)
    if ((uint32_t)page_directory & 0xFFF) {
        vga_print_string("[FAILED - Directory not aligned]\n");
        return;
    }
    if ((uint32_t)fpage_table & 0xFFF) {
        vga_print_string("[FAILED - Table not aligned]\n");
        return;
    }

    // Initialize page directory - clear all entries first
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    // Set up first page table for identity mapping (first 4MB)
    for (int i = 0; i < 1024; i++) {
        // Present, Read/Write, Kernel-mode, Frame address
        fpage_table[i] = (i * 0x1000) | 3; // 0x1000 = 4KB, 3 = Present + Read/Write
    }

    // Install first page table into page directory
    // Set the first entry of the page directory to point to the first_page_table
    // Flags: Present (1), Read/Write (1), Kernel-mode (0)
    page_directory[0] = ((uint32_t)fpage_table) | 3;

    // Load page directory into CR3
    asm volatile("mov %0, %%cr3" :: "r"(&page_directory) : "memory");

    // Enable paging by setting bit 31 in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");

    vga_print_string("[OK]\n");
}