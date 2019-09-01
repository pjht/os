#include <sys/types.h>

pid_t getpid() {
  pid_t pid;
  asm volatile("  \
    mov $20, %%eax; \
    int $80; \
  ":"=b"(pid));
  return pid;
}
