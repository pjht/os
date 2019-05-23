#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define BLK_SZ 4096

void* alloc_memory(uint32_t num_pages);
void alloc_memory_virt(uint32_t num_pages,void* addr);
void* new_address_space();
void copy_data(void* cr3, void* data,uint32_t size,void* virt_addr);
void* map_phys(void* phys_addr,uint32_t num_pages);

#endif
