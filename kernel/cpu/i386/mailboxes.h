#ifndef MALBOXES_H
#define MAILBOXES_H

typedef struct {
  void* msg;
  uint32_t sender;
  uint32_t size;
} Message;

typedef struct {
  uint32_t rd;
  uint32_t wr;
  uint16_t size;
  Message* msg_store;
} Mailbox;

#include "mailboxes.h"

Mailbox* mailbox_new(uint16_t size);
void mailbox_put_msg(Mailbox* mailbox, uint32_t pid,char* msg,uint32_t size);
void* mailbox_get_msg(Mailbox* mailbox, Mailbox** sender,uint32_t* size);

#endif
