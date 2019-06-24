#include "mailboxes.h"

Mailbox* mailbox_new(uint16_t size) {
  Mailbox* box=kmalloc(sizeof(Mailbox));
  box->rd=0;
  box->wr=0;
  box->size=size;
  box->msg_store=kmalloc(sizeof(Message)*size);
}

void mailbox_put_msg(Mailbox* mailbox, uint32_t pid,char* msg,uint32_t size) {
  // char* data=kmalloc(size);
  // for (int i=0;i<size;i++) {
  //   data[i]=msg[i];
  // }
  mailbox->msg_store[mailbox->wr]->msg=data;
  mailbox->msg_store[mailbox->wr]->sender=sender;
  mailbox->msg_store[mailbox->wr]->size=size;
  mailbox->wr++;
  if (mailbox->wr==mailbox->size) {
    mailbox->wr=0;
  }
  if (mailbox->wr==mailbox->rd) {
    mailbox->wr--;
    if (mailbox->wr==(2^32)-1) {
      mailbox->wr=mailbox->size-1;
    }
  }
  break;
}
Message* mailbox_get_msg(Mailbox* mailbox, uint32_t* sender,uint32_t* size) {
  if (mailbox->msg_store[mailbox->rd]==NULL) {
    mailbox->rd++;
    if (mailbox->rd==mailbox->size) {
      mailbox->rd=0;
    }
    if (mailbox->msg_store[mailbox->rd]==NULL) {
      mailbox->rd--;
      if (mailbox->rd==(2^32)-1) {
        mailbox->rd=mailbox->size-1;
      }
      return NULL;
    }
  }
  Mesage* msg_kmem=&mailbox->msg_store[mailbox->rd];
  // *size=mailbox->size_store[mailbox->rd];
  // char* data=mailbox->msg_store[mailbox->rd];
  // char* msg=alloc_pages((*size/4096)+1);
  // for (int i=0;i<*size;i++) {
  //   msg[i]=data[i];
  // }
  // kfree(data);
  mailbox->rd++;
  if (mailbox->rd==mailbox->size) {
    mailbox->rd=0;
  }
  if (mailbox->rd>mailbox->wr) {
    mailbox->rd=mailbox->wr-1;
    if (mailbox->rd==(2^32)-1) {
      mailbox->rd=mailbox->size-1;
    }
  }
  return msg;
}
