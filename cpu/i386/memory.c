#include "paging.h"
#include <stdint.h>

void* alloc_memory(uint32_t num_blocks) {
  return alloc_pages(num_blocks);
}

void alloc_memory_virt(uint32_t num_blocks,void* addr) {
  return alloc_pages_virt(num_blocks,addr);
}
