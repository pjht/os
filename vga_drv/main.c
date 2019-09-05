#include <grub/text_fb_info.h>
#include <mailboxes.h>
#include <ipc/devfs.h>
#include <ipc/vfs.h>
#include <string.h>
#include <stdlib.h>
#include <tasking.h>
#include <memory.h>
#include "vga.h"
#include <dbg.h>

uint32_t devfs_box;
#define DEVFS_PID 3

int main() {
  uint32_t box=mailbox_new(16,"vga");
  devfs_box=mailbox_find_by_name("devfs_devfs");
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  yield();
  devfs_message* msg_data=malloc(sizeof(devfs_message));
  msg_data->msg_type=DEVFS_REGISTER;
  msg_data->dev_type=DEV_GLOBAL;
  msg_data->mbox=box;
  strcpy(&msg_data->name[0],"vga");
  Message msg;
  msg.from=box;
  msg.to=devfs_box;
  msg.size=sizeof(devfs_message);
  msg.msg=msg_data;
  mailbox_send_msg(&msg);
  free(msg_data);
  yieldToPID(DEVFS_PID);
  for (;;) {
    Message msg;
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      free(msg.msg);
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      char* gets_data=NULL;
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
          serial_print("Could not recieve fputs data from the devfs\n");
          vfs_msg->flags=2;
          break;
        }
        vga_write_string(data);
        vfs_msg->flags=0;
        break;
        }
        case VFS_GETS: {
        vfs_msg->flags=2;
        break;
        }
        default:
        vfs_msg->flags=1;
      }
      msg.to=msg.from;
      msg.from=box;
      mailbox_send_msg(&msg);
      free(msg.msg);
      if (gets_data && vfs_msg->flags==0) {
        serial_print("GETS DATA VGA\n");
        msg.msg=gets_data;
        msg.size=vfs_msg->data;
        mailbox_send_msg(&msg);
        free(gets_data);
      } else {
        if (vfs_msg->type==VFS_GETS) {
          serial_print("NO GETS DATA VGA\n");
        }
      }
    }
    yield();
  }
}
