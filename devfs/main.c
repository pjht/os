#include <tasking.h>
#include <stdlib.h>
#include <mailboxes.h>
#include <ipc/vfs.h>
#include <memory.h>
#include <grub/text_fb_info.h>
#include "vga.h"
#include <vfs.h>

int main() {
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  uint32_t box=mailbox_new(16);
  register_fs("devfs",box);
  for (;;) {
    yield();
    Message msg;
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      yield();
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      switch (vfs_msg->type) {
        case VFS_OPEN:
        vfs_msg->flags=0;
        break;
        case VFS_PUTS: {
        char* data=malloc(sizeof(char)*vfs_msg->data);
        Message msg;
        msg.msg=data;
        mailbox_get_msg(box,&msg,vfs_msg->data);
        while (msg.from==0 && msg.size==0) {
          yield();
          mailbox_get_msg(box,&msg,sizeof(vfs_message));
        }
        if (msg.from==0) {
          serial_print("Could not recieve fputs data from the VFS\n");
          vfs_msg->flags=2;
          break;
        }
        vga_write_string(data);
        vfs_msg->flags=0;
        break;
        }
        case VFS_MOUNT: {
          char* disk_file=malloc(sizeof(char)*vfs_msg->data);
          Message msg;
          msg.msg=disk_file;
          mailbox_get_msg(box,&msg,vfs_msg->data);
          while (msg.from==0 && msg.size==0) {
            yield();
            mailbox_get_msg(box,&msg,sizeof(vfs_message));
          }
          if (msg.from==0) {
            serial_print("Could not recieve disk file path from the VFS\n");
            vfs_msg->flags=2;
            break;
          }
          free(disk_file);
          vfs_msg->flags=0;
          break;
        }
        default:
        vfs_msg->flags=1;
      }
      msg.to=msg.from;
      msg.from=box;
      mailbox_send_msg(&msg);
    }
    free(msg.msg);
  }
}
