#include "paging.h"
#include <stdint.h>

void* alloc_memory(uint32_t num_blocks) {
  return alloc_kern_pages(num_blocks,1);
}
