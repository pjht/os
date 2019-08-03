#include <tasking.h>
#include <stdlib.h>
#include <mailboxes.h>
#include <ipc/vfs.h>
#include <memory.h>
#include <grub/text_fb_info.h>
#include "vga.h"
#include <vfs.h>

int main() {
  register_fs("devfs");
  // mount("devfs","","/dev");
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
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
      char str[]={(char)vfs_msg->data,'\0'};
      vga_write_string(&str[0]);
      msg.to=msg.from;
      msg.from=box;
      vfs_msg->flags=0;
      mailbox_send_msg(&msg);
      yield();
    }
    free(msg.msg);
  }
}
