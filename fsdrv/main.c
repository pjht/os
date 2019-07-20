#include <tasking.h>
#include <stdlib.h>
#include <mailboxes.h>
#include <ipc/vfs.h>
#include <memory.h>

int main() {
  uint32_t box=mailbox_new(16);
  for (;;) {
    yield();
    Message msg;
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      yield();
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      msg.to=msg.from;
      msg.from=box;
      vfs_msg->flags=0;
      mailbox_send_msg(&msg);
      yield();
    }
    free(msg.msg);
  }
}
