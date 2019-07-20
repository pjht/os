#include "mailboxes.h"
#include "kmalloc.h"
#include <string.h>
#include <stdint.h>
#include <mailboxes.h>

Mailbox* mailboxes=(Mailbox*)0xF6400000;
uint32_t next_box=1;

uint32_t kernel_mailbox_new(uint16_t size) {
  if (next_box==262144) {
    return 0xFFFFFFFF;
  }
  mailboxes[next_box].rd=0;
  mailboxes[next_box].wr=0;
  mailboxes[next_box].size=size;
  mailboxes[next_box].msg_store=kmalloc(sizeof(Message)*size);
  next_box++;
  return next_box-1;
}

void kernel_mailbox_free(uint32_t box) {
  kfree(mailboxes[box].msg_store);
}

void kernel_mailbox_send_msg(Message* user_msg) {
  Mailbox mailbox=mailboxes[user_msg->to];
  char* msg_data=kmalloc(user_msg->size);
  memcpy(msg_data,user_msg->msg,user_msg->size);
  mailbox.msg_store[mailbox.wr].msg=msg_data;
  mailbox.msg_store[mailbox.wr].from=user_msg->from;
  mailbox.msg_store[mailbox.wr].to=user_msg->to;
  mailbox.msg_store[mailbox.wr].size=user_msg->size;
  mailbox.wr++;
  if (mailbox.wr==mailbox.size) {
    mailbox.wr=0;
  }
  if (mailbox.wr==mailbox.rd) {
    mailbox.wr--;
    if (mailbox.wr==(2^32)-1) {
      mailbox.wr=mailbox.size-1;
    }
  }
}

void kernel_mailbox_get_msg(uint32_t box, Message* recv_msg, uint32_t buffer_sz) {
  Mailbox mailbox=mailboxes[box];
  if (mailbox.msg_store[mailbox.rd].size==0) {
    mailbox.rd++;
    if (mailbox.rd==mailbox.size) {
      mailbox.rd=0;
    }
    if (mailbox.msg_store[mailbox.rd].size==0) {
      mailbox.rd--;
      if (mailbox.rd==(2^32)-1) {
        mailbox.rd=mailbox.size-1;
      }
      recv_msg->size=0;
      recv_msg->from=0;
      return;
    }
  }
  recv_msg->from=mailbox.msg_store[mailbox.rd].from;
  recv_msg->to=mailbox.msg_store[mailbox.rd].to;
  recv_msg->size=mailbox.msg_store[mailbox.rd].size;
  if (buffer_sz>mailbox.msg_store[mailbox.rd].size) {
    recv_msg->size=mailbox.msg_store[mailbox.rd].size;
    recv_msg->from=0;
    return;
  }
  memcpy(recv_msg->msg,mailbox.msg_store[mailbox.rd].msg,mailbox.msg_store[mailbox.rd].size);
  kfree(mailbox.msg_store[mailbox.rd].msg);
  mailbox.msg_store[mailbox.rd].size=0;
  mailbox.rd++;
  if (mailbox.rd==mailbox.size) {
    mailbox.rd=0;
  }
  if (mailbox.rd>mailbox.wr) {
    mailbox.rd=mailbox.wr-1;
    if (mailbox.rd==(2^32)-1) {
      mailbox.rd=mailbox.size-1;
    }
  }
}
