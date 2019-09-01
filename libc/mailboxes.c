#include <stdint.h>
#include <mailboxes.h>

uint32_t mailbox_new(uint16_t size,char* name) {
  uint32_t box;
  asm volatile("  \
    mov $14, %%eax; \
    int $80; \
  ":"=b"(box):"b"(size),"c"(name));
  return box;
}

void mailbox_send_msg(Message* msg) {
  asm volatile("  \
    mov $7, %%eax; \
    int $80; \
  "::"b"(msg));
}

void mailbox_get_msg(uint32_t box, Message* recv_msg, uint32_t buffer_sz)  {
  asm volatile("  \
    mov $6, %%eax; \
    int $80; \
  "::"b"(box),"c"(recv_msg),"d"(buffer_sz));
}
