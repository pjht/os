#include <stdint.h>

void* alloc_memory(uint32_t num_pages) {
  void* address;
  asm volatile("  \
    mov $3, %%eax; \
    int $80; \
  ":"=b"(address):"b"(num_pages));
  return address;
}

void alloc_memory_virt(uint32_t num_pages,void* addr) {
  void* address;
  asm volatile("  \
    mov $4, %%eax; \
    int $80; \
  "::"b"(num_pages),"c"(addr));
}
