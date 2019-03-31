#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define BLK_SZ 4096

void* alloc_memory(uint32_t num_pages);
void alloc_memory_virt(uint32_t num_pages,void* addr);

#endif
