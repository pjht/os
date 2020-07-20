#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void serial_print(char* str) {
  asm volatile("  \
    mov $" QU(SYSCALL_SERIAL_PRINT) ", %%eax; \
    int $80; \
  "::"b"(str));
}
