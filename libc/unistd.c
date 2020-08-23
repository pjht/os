#include <sys/syscalls.h>
#include <sys/types.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

pid_t getpid() {
  pid_t pid;
  asm volatile("  \
    mov $" QU(SYSCALL_GET_PID) ", %%eax; \
    int $80; \
  ":"=b"(pid));
  return pid;
}
