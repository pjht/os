#include <stdint.h>
#include <sys/syscalls.h>
#include <sys/types.h>
#include <tasking.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void yield() {
  asm volatile("  \
    mov $" QU(SYSCALL_YIELD) ", %%eax; \
    int $80; \
  "::"b"(0));
}

void createProcCr3(void* start,void* cr3) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC) ", %%eax; \
    int $80; \
  "::"b"(start),"d"(0),"c"(cr3));
}

void createProcCr3Param(void* start,void* cr3,uint32_t param1,uint32_t param2) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC) ", %%eax; \
    int $80; \
  "::"b"(start),"c"(cr3),"d"(1),"S"(param1),"D"(param2));
}

void yieldToPID(uint32_t pid) {
  asm volatile("  \
    mov $" QU(SYSCALL_YIELD) ", %%eax; \
    int $80; \
  "::"b"(pid));
}

__attribute__((noreturn)) void exit(int code) {
  code=code&0xff;
  asm volatile("  \
    mov $" QU(SYSCALL_EXIT) ", %%eax; \
    int $80; \
  "::"b"(code));
  for(;;);
}


void blockThread(ThreadState state) {
  asm volatile("  \
    mov $" QU(SYSCALL_BLOCK) ", %%eax; \
    int $80; \
  "::"b"(state));
}

void unblockThread(pid_t pid,uint32_t tid) {
  asm volatile("  \
    mov $" QU(SYSCALL_UNBLOCK) ", %%eax; \
    int $80; \
  "::"b"(pid),"c"(tid));
}
