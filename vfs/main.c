#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>

int main() {
  uint32_t box=mailbox_new(16);
  yield();
  Message msg;
  msg.msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  vfs_msg->fd=2;
  msg.to=msg.from;
  msg.from=box;
  mailbox_send_msg(&msg);
  // send_msg(1,msg,size);
  yield();
  for (;;);
}
