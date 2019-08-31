#include <tasking.h>
#include <stdlib.h>
#include <mailboxes.h>
#include <ipc/vfs.h>
#include <memory.h>
#include <grub/text_fb_info.h>
#include <vfs.h>
#include <ipc/devfs.h>
#include <string.h>

uint32_t vfs_box;
uint32_t devfs_box;
int num_devs;
int max_devs;
char** dev_names;
uint32_t* dev_mboxes;
int* dev_types;

void process_vfs_msg(vfs_message* vfs_msg) {
  switch (vfs_msg->type) {
    case VFS_OPEN: {
    int i;
    char found=0;
    for (i=0;i<num_devs;i++) {
      if (strcmp(dev_names[i],&vfs_msg->path[0])==0) {
        found=1;
        break;
      }
    }
    if (!found) {
      vfs_msg->flags=2;
      break;
    }
    if (dev_types[i]==1) {
      Message msg;
      msg.from=devfs_box;
      msg.to=dev_mboxes[i];
      msg.size=sizeof(vfs_message);
      msg.msg=vfs_msg;
      mailbox_send_msg(&msg);
      yield();
      vfs_message* recv_msg=malloc(sizeof(vfs_message));
      msg.msg=recv_msg;
      mailbox_get_msg(devfs_box,&msg,sizeof(vfs_message));
      while (msg.from==0 && msg.size==0) {
        yield();
        mailbox_get_msg(devfs_box,&msg,sizeof(vfs_message));
      }
      vfs_msg->flags=recv_msg->flags;
    } else {
      vfs_msg->flags=0;
    }
    if (vfs_msg->flags==0) {
      uint32_t* data=malloc(sizeof(uint32_t));
      *data=dev_mboxes[i];
      vfs_msg->fs_data=data;
    }
    break;
    }
    case VFS_PUTS: {
    char* data=malloc(sizeof(char)*vfs_msg->data);
    Message msg;
    msg.msg=data;
    mailbox_get_msg(vfs_box,&msg,vfs_msg->data);
    while (msg.from==0 && msg.size==0) {
      yield();
      mailbox_get_msg(vfs_box,&msg,sizeof(vfs_message));
    }
    if (msg.from==0) {
      serial_print("Could not recieve fputs data from the VFS\n");
      vfs_msg->flags=2;
      break;
    }
    msg.from=devfs_box;
    msg.to=*((int*)vfs_msg->fs_data);
    msg.size=sizeof(vfs_message);
    msg.msg=vfs_msg;
    mailbox_send_msg(&msg);
    msg.size=vfs_msg->data;
    msg.msg=data;
    mailbox_send_msg(&msg);
    yield();
    vfs_message* recv_msg=malloc(sizeof(vfs_message));
    msg.msg=recv_msg;
    mailbox_get_msg(devfs_box,&msg,sizeof(vfs_message));
    while (msg.from==0 && msg.size==0) {
      yield();
      mailbox_get_msg(devfs_box,&msg,sizeof(vfs_message));
    }
    vfs_msg->flags=recv_msg->flags;
    break;
    }
    case VFS_MOUNT: {
      char* disk_file=malloc(sizeof(char)*vfs_msg->data);
      Message msg;
      msg.msg=disk_file;
      mailbox_get_msg(vfs_box,&msg,vfs_msg->data);
      while (msg.from==0 && msg.size==0) {
        yield();
        mailbox_get_msg(vfs_box,&msg,sizeof(vfs_message));
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
}


int main() {
  vfs_box=mailbox_new(16);
  devfs_box=mailbox_new(16);
  register_fs("devfs",vfs_box);
  dev_names=malloc(sizeof(char*)*32);
  dev_mboxes=malloc(sizeof(uint32_t)*32);
  dev_types=malloc(sizeof(int)*32);
  for (;;) {
    yield();
    Message msg;
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(vfs_box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      yield();
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      process_vfs_msg(vfs_msg);
      msg.to=msg.from;
      msg.from=vfs_box;
      mailbox_send_msg(&msg);
    }
    free(msg.msg);
    yield();
    msg.msg=malloc(sizeof(devfs_message));
    mailbox_get_msg(devfs_box,&msg,sizeof(devfs_message));
    if (msg.from==0) {
      yield();
    } else {
      devfs_message* devfs_msg=(devfs_message*)msg.msg;
      if (num_devs==max_devs) {
        max_devs+=32;
        dev_names=realloc(dev_names,sizeof(char*)*max_devs);
        dev_mboxes=realloc(dev_mboxes,sizeof(uint32_t)*max_devs);
        dev_types=realloc(dev_types,sizeof(int)*max_devs);
      }
      dev_names[num_devs]=malloc(sizeof(char)*strlen(&devfs_msg->name[0]));
      strcpy(dev_names[num_devs],&devfs_msg->name[0]);
      dev_mboxes[num_devs]=devfs_msg->mbox;
      dev_types[num_devs]=devfs_msg->dev_type;
      num_devs++;
    }
    free(msg.msg);
  }
}
