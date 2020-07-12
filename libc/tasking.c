#include <stdint.h>
#include <tasking.h>
#include <sys/types.h>
#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void yield() {
  asm volatile("  \
    mov $" QU(SYSCALL_YIELD) ", %eax; \
    int $80; \
  ");
}

void createTaskCr3(void* task,void* cr3) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC_GIVEN_ADDR_SPACE) ", %%eax; \
    int $80; \
  "::"b"(task),"c"(cr3));
}

void createTaskCr3Param(void* task,void* cr3,uint32_t param1,uint32_t param2) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC_GIVEN_ADDR_SPACE_W_ARGS) ", %%eax; \
    int $80; \
  "::"b"(task),"c"(cr3),"d"(param1),"S"(param2));
}

void yieldToPID(uint32_t pid) {
  asm volatile("  \
    mov $" QU(SYSCALL_YIELD_TO_PID) ", %%eax; \
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


void blockTask(TaskState state) {
  asm volatile("  \
    mov $" QU(SYSCALL_BLOCK) ", %%eax; \
    int $80; \
  "::"b"(state));
}

void unblockTask(pid_t pid) {
  asm volatile("  \
    mov $" QU(SYSCALL_UNBLOCK) ", %%eax; \
    int $80; \
  "::"b"(pid));
}
