#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

int* __get_errno_address() {
  int* address;
  asm volatile("  \
    mov $" QU(SYSCALL_GET_ERRNO_ADDR) ", %%eax; \
    int $80; \
  ":"=b"(address):);
  return address;
}
