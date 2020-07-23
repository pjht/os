#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define BLK_SZ 4096

void* alloc_memory(int num_pages);
void alloc_memory_virt(int num_pages,void* addr);
void* new_address_space();
void copy_data(void* cr3, void* data,size_t size,void* virt_addr);
void* put_data(void* cr3, void* data,size_t size);
void* map_phys(void* phys_addr,size_t num_pages);

#endif
