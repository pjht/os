#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "paging_helpers.h"

#define NUM_KERN_DIRS 4

void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr);
int new_kstack();
void unmap_pages(void* start_virt,uint32_t num_pages);
void* alloc_pages(int num_pages);
void alloc_pages_virt(int num_pages,void* addr);
void paging_init();
void* paging_new_address_space();
void load_address_space(uint32_t cr3);
void* virt_to_phys(void* virt_addr);
void* find_free_pages(int num_pages);
void* find_free_pages_wstart(int num_pages,int start_page);
void load_smap(uint32_t cr3);
char make_protector(int page);
char is_in_protector(uint32_t* addr);

#endif
