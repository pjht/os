#include <sys/syscalls.h>
#include <pthread.h>
#include <stddef.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void exception_return() {
  asm volatile("  \
    mov $" QU(SYSCALL_EXCEPTION_RETURN) ", %%eax; \
    int $80; \
  ":);
  pthread_exit(NULL);
}
void register_exception_handler(void* handler) {
  asm volatile("  \
    mov $" QU(SYSCALL_REGISTER_EXCEPTION_HANDLER) ", %%eax; \
    int $80; \
  "::"b"(handler));
}
