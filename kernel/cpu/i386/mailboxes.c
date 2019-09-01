#include "mailboxes.h"
#include "kmalloc.h"
#include <string.h>
#include <stdint.h>
#include <mailboxes.h>
#include "serial.h"
#include "../tasking.h"

Mailbox* mailboxes=(Mailbox*)0xF6400000;
uint32_t next_box=1;

uint32_t kernel_mailbox_new(uint16_t size,char* name) {
  if (next_box==262144) {
    serial_printf("Attempted to create a mailbox, but failed\n");
    return 0xFFFFFFFF;
  }
  mailboxes[next_box].rd=0;
  mailboxes[next_box].wr=0;
  mailboxes[next_box].size=size;
  mailboxes[next_box].msg_store=kmalloc(sizeof(Message)*size);
  if (strlen(name)>19) {
    name[20]='\0';
  }
  strcpy(mailboxes[next_box].name,name);
  serial_printf("PID %d created mailbox %s\n",getPID(),mailboxes[next_box].name);
  next_box++;
  return next_box-1;
}

void kernel_mailbox_free(uint32_t box) {
  kfree(mailboxes[box].msg_store);
}

void kernel_mailbox_send_msg(Message* user_msg) {
  if (user_msg->to==0) {
    serial_printf("Box %s attempted to send to box 0!\n",mailboxes[user_msg->from].name);
    return;
  }
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
  mailboxes[user_msg->to]=mailbox;
  serial_printf("Message sent from box %s to box %s\n",mailboxes[user_msg->from].name,mailboxes[user_msg->to].name);
}

void kernel_mailbox_get_msg(uint32_t box, Message* recv_msg, uint32_t buffer_sz) {
  Mailbox mailbox=mailboxes[box];
  if (mailbox.msg_store[mailbox.rd].size==0) {
    recv_msg->size=0;
    recv_msg->from=0;
    serial_printf("Box %s attempted to get a message, but there were none.\n",mailboxes[box].name);
    mailboxes[box]=mailbox;
    return;
  }
  recv_msg->from=mailbox.msg_store[mailbox.rd].from;
  recv_msg->to=mailbox.msg_store[mailbox.rd].to;
  recv_msg->size=mailbox.msg_store[mailbox.rd].size;
  if (buffer_sz<mailbox.msg_store[mailbox.rd].size) {
    recv_msg->size=mailbox.msg_store[mailbox.rd].size;
    recv_msg->from=0;
    serial_printf("Box %s attempted to get the message from box %s, but the buffer was too small.\n",mailboxes[box].name,mailboxes[mailbox.msg_store[mailbox.rd].from].name);
    mailboxes[box]=mailbox;
    return;
  }
  memcpy(recv_msg->msg,mailbox.msg_store[mailbox.rd].msg,mailbox.msg_store[mailbox.rd].size);
  kfree(mailbox.msg_store[mailbox.rd].msg);
  mailbox.msg_store[mailbox.rd].size=0;
  uint32_t orig_rd=mailbox.rd;
  mailbox.rd++;
  if (mailbox.rd==mailbox.size) {
    mailbox.rd=0;
  }
  if (mailbox.rd>mailbox.wr && !(orig_rd>mailbox.wr)) {
    mailbox.rd=mailbox.wr;
  }
  serial_printf("Box %s got a message from box %s.\n",mailboxes[box].name,mailboxes[recv_msg->from].name);
  mailboxes[box]=mailbox;
}
