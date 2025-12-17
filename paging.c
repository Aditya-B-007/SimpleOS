#include "paging.h"
#include "pmm.h"
#include "vga.h"
#include <string.h>
#include "idt.h"
#include "graphics.h"
struct VbeInfoBlock {
    uint16_t attributes;
    uint8_t windowA, windowB;
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t segmentA, segmentB;
    uint32_t winFuncPtr;
    uint16_t pitch;
    uint16_t width, height;
    uint8_t wChar, yChar, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;
    uint32_t physbase;  // <--- This is what we need!
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed));

page_directory_t* page_directory = 0;

void paging_map(uint32_t phys, uint32_t virt, uint32_t flags) {
    (void)flags;
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (!page_directory->tables[pd_index]) {
        // Allocate a new page table
        page_table_t* new_table = (page_table_t*)pmm_alloc_page();
        memset(new_table, 0, sizeof(page_table_t));
        
        // Register it in the directory
        page_directory->tables[pd_index] = new_table;
        // The physical address of the new table is just its pointer (in identity map)
        page_directory->physical_tables[pd_index] = ((uint32_t)new_table) | 0x7; // Present, RW, User
    }

    page_table_t* table = page_directory->tables[pd_index];
    table->pages[pt_index].frame = phys >> 12;
    table->pages[pt_index].present = 1;
    table->pages[pt_index].rw = 1;
    table->pages[pt_index].user = 1;
}

void switch_page_directory(page_directory_t *dir) {
    uint32_t cr0;
    // Load CR3 with the address of the physical_tables array
    asm volatile("mov %0, %%cr3" :: "r"(&dir->physical_tables));
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
void page_fault_handler(registers_t *r) {
    (void)r;
    uint32_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
    vga_print_string("Page Fault at 0x");
    vga_print_hex(faulting_address);
    vga_print_string("\n");
    __asm__ __volatile__("cli");
    for(;;) { __asm__ __volatile__("hlt"); }
}
void paging_install(void) {
    page_directory = (page_directory_t*)pmm_alloc_page();
    memset(page_directory, 0, sizeof(page_directory_t));
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        paging_map(i, i, 0x3);
    }
    for (uint32_t i = 0x00400000; i < 0x01400000; i += PAGE_SIZE) {
        paging_map(i, i, 0x3);
    }
    struct VbeInfoBlock* info = (struct VbeInfoBlock*)0x8000;
    uint32_t fb_phys = info->physbase;
    
    // Safety check: if 0, something is wrong with bootloader, but we try anyway
    if (fb_phys != 0) {
        // Map 8MB to be safe (covers 1080p easily)
        for (uint32_t i = 0; i < 0x00800000; i += PAGE_SIZE) {
            paging_map(fb_phys + i, fb_phys + i, 0x3);
        }
    }
    register_interrupt_handler(14, page_fault_handler);
    switch_page_directory(page_directory);
}