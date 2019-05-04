#include <stdint.h>

void yield() {
  asm volatile("  \
    mov $1, %eax; \
    int $80; \
  ");
}

void createTask(void* task) {
  asm volatile("  \
    mov $2, %%eax; \
    int $80; \
  "::"b"(task));
}

void* get_msg(uint32_t* sender) {
  void* msg;
  asm volatile("  \
    mov $6, %%eax; \
    int $80; \
  ":"=b"(msg):"b"(sender));
  return msg;
}

void send_msg(uint32_t pid,void* msg,uint32_t size) {
  asm volatile("  \
    mov $7, %%eax; \
    int $80; \
  "::"b"(pid),"c"(msg),"d"(size));
}
