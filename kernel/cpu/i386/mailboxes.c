#include "mailboxes.h"
#include "kmalloc.h"
#include <string.h>
#include <stdint.h>
#include <mailboxes.h>
#include "serial.h"
#include "paging.h"
#include "pmem.h"
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
  uint32_t num_pages=(user_msg->size/4096)+1;
  void* phys_addr=pmem_alloc(num_pages);
  void* virt_addr=find_free_pages(num_pages);
  // char* msg_data=kmalloc(user_msg->size);
  map_pages(virt_addr,phys_addr,num_pages,0,1);
  // if (msg_data==NULL) {
  //   serial_print("Cannot allocate kernel msg data!\n");
  //   vga_write_string("Cannot allocate kernel msg data!\n");
  //   for(;;);
  // }
  memcpy(virt_addr,user_msg->msg,user_msg->size);
  unmap_pages(virt_addr,num_pages);
  mailbox.msg_store[mailbox.wr].msg=phys_addr;
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
  if (mailbox.msg_store[mailbox.rd].from==0) {
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
    serial_printf("Expected message at most %d big, but got message sized %d.\n",buffer_sz,mailbox.msg_store[mailbox.rd].size);
    mailboxes[box]=mailbox;
    return;
  }
  Message msg=mailbox.msg_store[mailbox.rd];
  uint32_t num_pages=(msg.size/4096)+1;
  void* virt_addr=find_free_pages(num_pages);
  map_pages(virt_addr,msg.msg,num_pages,0,1);
  memcpy(recv_msg->msg,virt_addr,mailbox.msg_store[mailbox.rd].size);
  unmap_pages(virt_addr,num_pages);
  // kfree(mailbox.msg_store[mailbox.rd].msg);
  mailbox.msg_store[mailbox.rd].from=0;
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

uint32_t kernel_mailbox_find_by_name(char* name) {
  for (uint32_t i=1;i<next_box;i++) {
    if (strcmp(mailboxes[i].name,name)==0) {
      return i;
    }
  }
  return 0;
}
