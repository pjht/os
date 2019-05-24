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

void copy_data(void* cr3, void* data,uint32_t size,void* virt_addr) {
  asm volatile("  \
    mov $10, %%eax; \
    int $80; \
  "::"b"(cr3),"c"(data),"d"(size),"S"(virt_addr));
}

void* put_data(void* cr3, void* data,uint32_t size) {
  void* virt_addr;
  asm volatile("  \
    mov $13, %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(cr3),"c"(data),"d"(size));
  return virt_addr;
}

void* map_phys(void* phys_addr,uint32_t num_pages) {
  void* virt_addr;
  asm volatile("  \
    mov $11, %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(phys_addr),"c"(num_pages));
  return virt_addr;
}
