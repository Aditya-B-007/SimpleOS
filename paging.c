//Identity-map the first 4MB of physical memory
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
        page_directory[i].present = 0;
        page_directory[i].rw = 0;
        page_directory[i].user_supervisor = 0;
        page_directory[i].pwt = 0;
        page_directory[i].pcd = 0;
        page_directory[i].accessed = 0;
        page_directory[i].dirty = 0;
        page_directory[i].reserved = 0;
        page_directory[i].frame = 0;
    }

    // Set up first page table for identity mapping (first 4MB)
    for (int i = 0; i < 1024; i++) {
        fpage_table[i].present = 1;
        fpage_table[i].rw = 1;        // Read-write
        fpage_table[i].user_supervisor = 0; // Kernel mode
        fpage_table[i].pwt = 0;       // Write-back caching
        fpage_table[i].pcd = 0;       // Cacheable
        fpage_table[i].accessed = 0;
        fpage_table[i].dirty = 0;
        fpage_table[i].reserved = 0;
        fpage_table[i].frame = i;     // Identity map: virtual = physical
    }

    // Install first page table into page directory
    page_directory[0].frame = (uint32_t)fpage_table >> 12;
    page_directory[0].present = 1;
    page_directory[0].rw = 1;
    page_directory[0].user_supervisor = 0; // Kernel mode only
    page_directory[0].pwt = 0;
    page_directory[0].pcd = 0;
    page_directory[0].accessed = 0;
    page_directory[0].dirty = 0;
    page_directory[0].reserved = 0;

    // Load page directory into CR3
    asm volatile("mov %0, %%cr3" :: "r"(&page_directory) : "memory");

    // Enable paging by setting bit 31 in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit

    // Critical: Enable paging with error recovery
    __asm__ __volatile__(
        "mov %0, %%cr0\n"
        "jmp 1f\n"        // Force instruction fetch after paging enable
        "1:\n"
        :: "r"(cr0) : "memory"
    );

    // Verify paging is enabled
    uint32_t test_cr0;
    asm volatile("mov %%cr0, %0" : "=r"(test_cr0));
    if (!(test_cr0 & 0x80000000)) {
        vga_print_string("[FAILED - Paging not enabled]\n");
        return;
    }

    vga_print_string("[OK]\n");
}

// Return the start address of paging structures for PMM coordination
uint32_t paging_get_reserved_start(void) {
    return (uint32_t)&page_directory;
}

// Return the end address of paging structures for PMM coordination
uint32_t paging_get_reserved_end(void) {
    return (uint32_t)&fpage_table + sizeof(fpage_table);
}