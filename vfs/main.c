#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>
#include <stdlib.h>

vfs_message* get_message(Message* msg,uint32_t box) {
  msg->msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,msg,sizeof(vfs_message));
  vfs_message* vfs_msg=(vfs_message*)msg->msg;
  msg->to=msg->from;
  msg->from=box;
  return vfs_msg;
}

int main() {
  uint32_t box=mailbox_new(16);
  yield();
  Message msg;
  vfs_message* vfs_msg=get_message(&msg,box);
  vfs_msg->fd=2;
  mailbox_send_msg(&msg);
  yield();
  for (;;);
}
