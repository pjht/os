#include <rpc.h>
#include <sys/syscalls.h>

#define QUAUX(X) #X
#define QU(X) QUAUX(X)


void* rpc_call(pid_t pid,char* name,void* buf,size_t size) {
  void* retbuf;
  asm volatile("  \
    mov $" QU(SYSCALL_CALL_RPC) ", %%eax; \
    int $80; \
  ":"=D"(retbuf):"b"(pid),"c"(name),"d"(buf),"S"(size));
  return retbuf;
}

void rpc_register_func(char* name,rpc_func code) {
  asm volatile("  \
    mov $" QU(SYSCALL_REGISTER_RPC) ", %%eax; \
    int $80; \
  "::"b"(name),"c"(code));
}

void rpc_deallocate_buf(void* buf,size_t size) {
  asm volatile("  \
    mov $" QU(SYSCALL_DEALLOCTATE_RPC_RET) ", %%eax; \
    int $80; \
  "::"b"(buf),"c"(size));
}

void rpc_return(void* buf,size_t size) {
  asm volatile("  \
    mov $" QU(SYSCALL_RPC_RET) ", %%eax; \
    int $80; \
  "::"b"(buf),"c"(size));
}

void rpc_mark_as_init() {
  asm volatile("  \
    mov $" QU(SYSCALL_RPC_MARK_AS_INIT) ", %%eax; \
    int $80; \
  "::);
}
