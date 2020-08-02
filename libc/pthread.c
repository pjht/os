#include <pthread.h>
#include <stddef.h>
#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg) {
  if (thread==NULL) {
    return 1;
  }
  asm volatile("  \
    mov $" QU(SYSCALL_NEW_THREAD) ", %%eax; \
    int $80; \
  "::"b"(start_routine),"c"(thread),"d"(arg));
  return 0;
}

void pthread_exit(void *value_ptr) {
  asm volatile("  \
    mov $" QU(SYSCALL_THREAD_EXIT) ", %eax; \
    int $80;");
}
