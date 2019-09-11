#include <tasking.h>
#include <stdlib.h>
#include <mailboxes.h>
#include <ipc/vfs.h>
#include <memory.h>
#include <grub/text_fb_info.h>
#include <vfs.h>
#include <ipc/devfs.h>
#include <string.h>
#include <dbg.h>

uint32_t vfs_box;
uint32_t devfs_vfs_box;
uint32_t devfs_reg_box;
uint32_t devfs_drv_box;
int num_devs;
int max_devs;
char** dev_names;
uint32_t* dev_mboxes;
int* dev_types;

#define VFS_PID 2

char devfs_open(vfs_message* vfs_msg) {
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
    return 1;
  }
  if (dev_types[i]==1) {
    uint32_t* data=malloc(sizeof(uint32_t));
    *data=dev_mboxes[i];
    vfs_msg->fs_data=data;
    Message msg;
    msg.from=devfs_drv_box;
    msg.to=dev_mboxes[i];
    msg.size=sizeof(vfs_message);
    msg.msg=vfs_msg;
    mailbox_send_msg(&msg);
  } else {
    vfs_msg->flags=0;
    uint32_t* data=malloc(sizeof(uint32_t));
    *data=dev_mboxes[i];
    vfs_msg->fs_data=data;
    return 1;
  }
  vfs_msg->flags=2;
  return 1;
}

char devfs_puts(vfs_message* vfs_msg) {
  char* data=malloc(sizeof(char)*vfs_msg->data);
  Message msg;
  msg.msg=data;
  mailbox_get_msg(devfs_vfs_box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(devfs_vfs_box,&msg,sizeof(vfs_message));
  }
  if (msg.from==0) {
    serial_print("Could not recieve fputs data from the VFS\n");
    vfs_msg->flags=2;
    return 1;
  }
  msg.from=devfs_drv_box;
  msg.to=*((uint32_t*)vfs_msg->fs_data);
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  msg.size=vfs_msg->data;
  msg.msg=data;
  mailbox_send_msg(&msg);
  return 0;
}

char devfs_gets(vfs_message* vfs_msg) {
  Message msg;
  msg.from=devfs_drv_box;
  msg.to=*((uint32_t*)vfs_msg->fs_data);
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  vfs_msg->flags=0;
  return 0;
}

char* devfs_gets_finish(vfs_message* vfs_msg) {
  if (vfs_msg->flags) {
    return NULL;
  }
  char* gets_data=malloc(sizeof(vfs_msg->data));
  Message msg;
  msg.msg=gets_data;
  mailbox_get_msg(devfs_drv_box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(devfs_drv_box,&msg,sizeof(vfs_message));
  }
  vfs_msg->flags=0;
  return gets_data;
}

char devfs_mount(vfs_message* vfs_msg) {
  char* disk_file=malloc(sizeof(char)*vfs_msg->data);
  Message msg;
  msg.msg=disk_file;
  mailbox_get_msg(devfs_vfs_box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(devfs_vfs_box,&msg,sizeof(vfs_message));
  }
  if (msg.from==0) {
    serial_print("Could not recieve disk file path from the VFS\n");
    vfs_msg->flags=2;
    return 1;
  }
  free(disk_file);
  vfs_msg->flags=0;
  return 1;
}

void process_vfs_msg(vfs_message* vfs_msg, uint32_t from) {
  if (vfs_msg->in_progress&2) {
    if (vfs_msg->flags) {
      if (vfs_msg->type==VFS_OPEN) {
        free((void*)vfs_msg->data);
      }
      Message msg;
      msg.to=vfs_box;
      msg.from=devfs_vfs_box;
      msg.size=sizeof(vfs_message);
      msg.msg=vfs_msg;
      mailbox_send_msg(&msg);
    } else {
      char* gets_data=NULL;
      switch (vfs_msg->type) {
        case VFS_OPEN:
        vfs_msg->flags=0;
        break;
        case VFS_PUTS:
        vfs_msg->flags=0;
        break;
        case VFS_GETS:
        gets_data=devfs_gets_finish(vfs_msg);
        break;
        case VFS_MOUNT:
        vfs_msg->flags=0;
        break;
        default:
        vfs_msg->flags=1;
      }
      Message msg;
      msg.to=vfs_box;
      msg.from=devfs_vfs_box;
      msg.size=sizeof(vfs_message);
      msg.msg=vfs_msg;
      mailbox_send_msg(&msg);
      if (gets_data) {
        msg.size=vfs_msg->data;
        msg.msg=gets_data;
        mailbox_send_msg(&msg);
      }
    }
  } else {
    char send_msg;
    vfs_msg->in_progress=vfs_msg->in_progress|2;
    switch (vfs_msg->type) {
      case VFS_OPEN:
      send_msg=devfs_open(vfs_msg);
      break;
      case VFS_PUTS:
      send_msg=devfs_puts(vfs_msg);
      break;
      case VFS_GETS:
      send_msg=devfs_gets(vfs_msg);
      break;
      case VFS_MOUNT:
      serial_print("Mount\n");
      send_msg=devfs_mount(vfs_msg);
      break;
      default:
      vfs_msg->flags=1;
    }
    if (send_msg) {
      serial_print("Send message prep\n");
      Message msg;
      msg.to=vfs_box;
      msg.from=devfs_vfs_box;
      msg.size=sizeof(vfs_message);
      msg.msg=vfs_msg;
      serial_print("Send message\n");
      mailbox_send_msg(&msg);
      serial_print("Done\n");
    }
  }
  free(vfs_msg);
  yieldToPID(VFS_PID);
}


int main() {
  vfs_box=mailbox_find_by_name("vfs");
  devfs_vfs_box=mailbox_new(16,"devfs_vfs");
  devfs_reg_box=mailbox_new(16,"devfs_register");
  devfs_drv_box=mailbox_new(16,"devfs_driver");
  register_fs("devfs",devfs_vfs_box);
  dev_names=malloc(sizeof(char*)*32);
  dev_mboxes=malloc(sizeof(uint32_t)*32);
  dev_types=malloc(sizeof(int)*32);
  for (;;) {
    Message msg;
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(devfs_vfs_box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      free(msg.msg);
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      process_vfs_msg(vfs_msg,msg.from);
    }
    mailbox_get_msg(devfs_drv_box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      free(msg.msg);
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      process_vfs_msg(vfs_msg,msg.from);
    }
    msg.msg=malloc(sizeof(devfs_message));
    mailbox_get_msg(devfs_reg_box,&msg,sizeof(devfs_message));
    if (msg.from!=0) {
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
    yield();
  }
}
