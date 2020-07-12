#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

long initrd_sz() {
  long size;
  asm volatile("  \
    mov $" QU(SYSCALL_GET_INITRD_SZ) ", %%eax; \
    int $80; \
  ":"=b"(size));
  return size;
}

void initrd_get(char* initrd) {
  asm volatile("  \
    mov $" QU(SYSCALL_COPY_INITRD) ", %%eax; \
    int $80; \
  "::"b"(initrd));
}
