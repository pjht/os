#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "paging_helpers.h"

#define NUM_KERN_DIRS 2

void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr);
void* alloc_pages(int num_pages);
void alloc_pages_virt(int num_pages,void* addr);
void paging_init();


#endif
