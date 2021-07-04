#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void serial_print(char* str) {
  asm volatile("  \
    mov $" QU(SYSCALL_SERIAL_PRINT) ", %%eax; \
    int $80; \
  "::"b"(str));
}

void user_serial_putc(char c, int port) {
  asm volatile("  \
    mov $" QU(SYSCALL_SERIAL_PUTC) ", %%eax; \
    int $80; \
  "::"b"(c),"c"(port));
}

char user_serial_getc(int port) {
  char c;
  asm volatile("  \
    mov $" QU(SYSCALL_SERIAL_GETC) ", %%eax; \
    int $80; \
  ":"=b"(c):"b"(port));
  return c;
}
