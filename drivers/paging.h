#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "paging_helpers.h"

#define NUM_KERN_DIRS 2
#define KERN_VIRT_START 0xC0100000
#define KERN_PHYS_START 0x100000
void alloc_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr,uint32_t* page_directory,uint32_t* page_tables);
void* alloc_kern_pages(int num_pages,char wr);
int dir_entry_present(int entry);
void* virt_to_phys(void* virt_addr);
void initialize_paging();


#endif
