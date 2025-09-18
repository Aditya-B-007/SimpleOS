//Identity-map the first 4MB of physical memory
#include "paging.h"
#include "vga.h"
//Static allocation currently, 4MB=4096-byte aligned
__attribute__((aligned(4096))) pt_entry_t page_directory[1024];
__attribute__((aligned(4096))) pt_entry_t fpage_table[1024];
void paging_install(void){
    vga_print_string("Installing paging... ");
    for(int i=0;i<1024;i++){
        fpage_table[i].present=1;
        fpage_table[i].user_supervisor=0;
        fpage_table[i].pwt=0;
        fpage_table[i].pcd=0;
        fpage_table[i].accessed=0;
        fpage_table[i].reserved=0;
        fpage_table[i].rw=1; //Read-write
        fpage_table[i].frame=i;
}
//Newly created page goes into the first entry of the page directoy
page_directory[0].frame=(uint32_t)fpage_table>>12;
page_directory[0].present=1;
page_directory[0].rw=1;
asm volatile("mov %0, %%cr3"::"r"(&page_directory));
uint32_t cr0;
asm volatile("mov %%cr0, %0":"=r"(cr0));
cr0|=0x80000000;
asm volatile("mov %0, %%cr0"::"r"(cr0));
vga_print_string("[OK]\n");
}
