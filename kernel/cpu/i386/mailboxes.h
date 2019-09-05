#ifndef KERNEL_MAILBOXES_H
#define KERNEL_MAILBOXES_H

#include <stdint.h>
#include <mailboxes.h>

uint32_t kernel_mailbox_new(uint16_t size,char* name);
void kernel_mailbox_free(uint32_t box);
void kernel_mailbox_send_msg(Message* user_msg);
void kernel_mailbox_get_msg(uint32_t box, Message* recv_msg, uint32_t buffer_sz);
uint32_t kernel_mailbox_find_by_name(char* name);

#endif
