#include <sys/syscalls.h>
#define QUAUX(X) #X
#define QU(X) QUAUX(X)

void register_irq_handler(int irq_no,void* handler) {
  asm volatile("  \
    mov $" QU(SYSCALL_REGISTER_IRQ_HANDLER) ", %%eax; \
    int $80; \
  "::"b"(irq_no),"c"(handler));
}
