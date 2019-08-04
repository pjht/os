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

void createTaskCr3(void* task,void* cr3) {
  asm volatile("  \
    mov $9, %%eax; \
    int $80; \
  "::"b"(task),"c"(cr3));
}

void createTaskCr3Param(void* task,void* cr3,uint32_t param1,uint32_t param2) {
  asm volatile("  \
    mov $12, %%eax; \
    int $80; \
  "::"b"(task),"c"(cr3),"d"(param1),"S"(param2));
}

void yieldToPID(uint32_t pid) {
  asm volatile("  \
    mov $15, %%eax; \
    int $80; \
  "::"b"(pid));
}
