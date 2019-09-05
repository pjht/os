#ifndef MAILBOXES_H
#define MAILBOXES_H
#include <stdint.h>


typedef struct {
  void* msg;
  uint32_t from;
  uint32_t to;
  uint32_t size;
} Message;

typedef struct {
  uint32_t rd;
  uint32_t wr;
  uint16_t size;
  Message* msg_store;
  char name[20];
} Mailbox;

uint32_t mailbox_new(uint16_t size,char* name);
void mailbox_send_msg(Message* msg);
void mailbox_get_msg(uint32_t box, Message* recv_msg, uint32_t buffer_sz);
uint32_t mailbox_find_by_name(char* name);

#endif
