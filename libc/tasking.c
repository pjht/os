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

void create_proc(void* start,void* address_space) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC) ", %%eax; \
    int $80; \
  "::"b"(start),"d"(0),"c"(address_space));
}

void create_proc_param(void* start,void* address_space,void* param1,void* param2) {
  asm volatile("  \
    mov $" QU(SYSCALL_CREATEPROC) ", %%eax; \
    int $80; \
  "::"b"(start),"c"(address_space),"d"(1),"S"(param1),"D"(param2));
}

__attribute__((noreturn)) void exit(int code) {
  code=code&0xff;
  asm volatile("  \
    mov $" QU(SYSCALL_EXIT) ", %%eax; \
    int $80; \
  "::"b"(code));
  for(;;);
}


void block_thread(thread_state state) {
  asm volatile("  \
    mov $" QU(SYSCALL_BLOCK) ", %%eax; \
    int $80; \
  "::"b"(state));
}

void unblock_thread(pid_t pid,pid_t tid) {
  asm volatile("  \
    mov $" QU(SYSCALL_UNBLOCK) ", %%eax; \
    int $80; \
  "::"b"(pid),"c"(tid));
}
