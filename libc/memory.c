#include <stdint.h>
#include <stdlib.h>
#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void* alloc_memory(uint32_t num_pages) {
  void* address;
  asm volatile("  \
    mov $" QU(SYSCALL_ALLOC_MEM) ", %%eax; \
    int $80; \
  ":"=b"(address):"b"(num_pages),"c"(NULL));
  return address;
}

void alloc_memory_virt(uint32_t num_pages,void* addr) {
  asm volatile("  \
    mov $" QU(SYSCALL_ALLOC_MEM) ", %%eax; \
    int $80; \
  "::"b"(num_pages),"c"(addr));
}

void* new_address_space() {
  void* cr3;
  asm volatile("  \
    mov $" QU(SYSCALL_NEW_ADDR_SPACE) ", %%eax; \
    int $80; \
  ":"=b"(cr3));
  return cr3;
}

void copy_data(void* cr3, void* data,uint32_t size,void* virt_addr) {
  asm volatile("  \
    mov $" QU(SYSCALL_ADDR_SPACES_COPY_DATA) ", %%eax; \
    int $80; \
  "::"b"(cr3),"c"(data),"d"(size),"S"(virt_addr));
}

void* put_data(void* cr3, void* data,uint32_t size) {
  void* virt_addr;
  asm volatile("  \
    mov $" QU(SYSCALL_ADDR_SPACES_COPY_DATA) ", %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(cr3),"c"(data),"d"(size),"S"(NULL));
  return virt_addr;
}

void* map_phys(void* phys_addr,uint32_t num_pages) {
  void* virt_addr;
  asm volatile("  \
    mov $" QU(SYSCALL_PRIV_MAP_PAGES) ", %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(phys_addr),"c"(num_pages));
  return virt_addr;
}
