#include <stdint.h>
#include <stdlib.h>
#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void* alloc_memory(int num_pages) {
  void* address;
  asm volatile("  \
    mov $" QU(SYSCALL_ALLOC_MEM) ", %%eax; \
    int $80; \
  ":"=b"(address):"b"(num_pages),"c"(NULL));
  return address;
}

void alloc_memory_virt(int num_pages,void* addr) {
  asm volatile("  \
    mov $" QU(SYSCALL_ALLOC_MEM) ", %%eax; \
    int $80; \
  "::"b"(num_pages),"c"(addr));
}

void* new_address_space() {
  void* address_space;
  asm volatile("  \
    mov $" QU(SYSCALL_NEW_ADDR_SPACE) ", %%eax; \
    int $80; \
  ":"=b"(address_space));
  return address_space;
}

void copy_data(void* address_space, void* data,size_t size,void* virt_addr) {
  asm volatile("  \
    mov $" QU(SYSCALL_ADDR_SPACES_COPY_DATA) ", %%eax; \
    int $80; \
  "::"b"(address_space),"c"(data),"d"(size),"S"(virt_addr));
}

void* put_data(void* address_space, void* data,size_t size) {
  void* virt_addr;
  asm volatile("  \
    mov $" QU(SYSCALL_ADDR_SPACES_COPY_DATA) ", %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(address_space),"c"(data),"d"(size),"S"(NULL));
  return virt_addr;
}

void* map_phys(void* phys_addr,size_t num_pages) {
  void* virt_addr;
  asm volatile("  \
    mov $" QU(SYSCALL_PRIV_MAP_PAGES) ", %%eax; \
    int $80; \
  ":"=b"(virt_addr):"b"(phys_addr),"c"(num_pages));
  return virt_addr;
}
