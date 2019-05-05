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
  asm volatile("  \
    mov $4, %%eax; \
    int $80; \
  "::"b"(num_pages),"c"(addr));
}

void* new_address_space() {
  void* cr3;
  asm volatile("  \
    mov $8, %%eax; \
    int $80; \
  ":"=b"(cr3));
  return cr3;
}
