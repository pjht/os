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
  free(data);
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
  char str[256];
  int_to_ascii(vfs_msg->data,str);
  serial_print("gets_finish: Driver wants ");
  serial_print(str);
  serial_print(" bytes of gets data\n");
  int_to_ascii(vfs_msg->type,str);
  serial_print("gets_finish: Driver has message type ");
  serial_print(str);
  serial_print("\n");
  if (vfs_msg->flags) {
    return NULL;
  }
  uint8_t* gets_data=malloc(sizeof(uint8_t)*vfs_msg->data);
  Message msg;
  msg.msg=gets_data;
  int_to_ascii(vfs_msg->data,str);
  serial_print("gets_finish: Driver now wants ");
  serial_print(str);
  serial_print(" bytes of gets data\n");
  msg.msg=gets_data;
  int_to_ascii(vfs_msg->type,str);
  serial_print("gets_finish: Driver now has message type ");
  serial_print(str);
  serial_print("\n");

  // int_to_ascii(devfs_drv_box,str);
  // serial_print("mailbox_get_msg(");
  // serial_print(str);
  // serial_print(",");
  // hex_to_ascii(msg,str);
  // serial_print(str);
  // serial_print(",");
  // int_to_ascii((uint32_t)vfs_msg->data,str);
  // serial_print(str);
  // serial_print(");\n");
  vfs_message temp_msg;
  memcpy(&temp_msg, vfs_msg, sizeof(vfs_message));
  mailbox_get_msg(devfs_drv_box,&msg,(uint32_t)vfs_msg->data);
  // while (msg.from==0 && msg.size==0) {
  //   serial_print("Yielding to wait for data msg\n");
  //   yield();
  //   mailbox_get_msg(devfs_drv_box,&msg,(uint32_t)vfs_msg->data);
  // }
  memcpy(vfs_msg, &temp_msg, sizeof(vfs_message));
  int_to_ascii(vfs_msg->data,str);
  serial_print("gets_finish: Again, driver now wants ");
  serial_print(str);
  serial_print(" bytes of gets data\n");
  int_to_ascii(vfs_msg->type,str);
  serial_print("gets_finish: Again, driver now has message type ");
  serial_print(str);
  serial_print("\n");
  vfs_msg->flags=0;
  int_to_ascii(vfs_msg->data,str);
  serial_print("gets_finish: Sending ");
  serial_print(str);
  serial_print(" bytes of gets data\n");
  int_to_ascii(vfs_msg->type,str);
  serial_print("gets_finish: Sending message type ");
  serial_print(str);
  serial_print("\n");
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
  char str[256];
  int_to_ascii(vfs_msg->data,str);
  serial_print("vfs_msg->data=");
  serial_print(str);
  serial_print("\n");
  if (from!=vfs_box) {
    if (vfs_msg->flags) {
      serial_print("vfs_msg->flags=");
      char str[256];
      int_to_ascii(vfs_msg->flags,str);
      serial_print(str);
      serial_print("\n");
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
      serial_print("GOT FINISH TYPE ");
      char str[256];
      int_to_ascii(vfs_msg->type,str);
      serial_print(str);
      serial_print("\n");
      switch (vfs_msg->type) {
        case VFS_OPEN:
        vfs_msg->flags=0;
        break;
        case VFS_PUTS:
        vfs_msg->flags=0;
        break;
        case VFS_GETS:
        int_to_ascii(vfs_msg->data,str);
        serial_print("process_vfs_msg: Driver sending ");
        serial_print(str);
        serial_print(" bytes of gets data\n");
        serial_print("GETS_FINISH\n");
        gets_data=devfs_gets_finish(vfs_msg);
        serial_print("GETS_FINISH DONE\n");
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
      serial_print("SENDING FINISH TYPE ");
      int_to_ascii(vfs_msg->type,str);
      serial_print(str);
      serial_print("\n");
      mailbox_send_msg(&msg);
      if (gets_data) {
        serial_print("Gets data\n");
        char str[256];
        int_to_ascii(vfs_msg->data,str);
        serial_print("Sending ");
        serial_print(str);
        serial_print(" bytes of gets data\n");
        msg.size=vfs_msg->data;
        msg.msg=gets_data;
        mailbox_send_msg(&msg);
        free(gets_data);
      } else {
        serial_print("No gets data\n");
      }
    }
  } else {
    char send_msg;
    switch (vfs_msg->type) {
      case VFS_OPEN:
      serial_print("OPEN\n");
      send_msg=devfs_open(vfs_msg);
      serial_print("OPEN DONE\n");
      break;
      case VFS_PUTS:
      serial_print("PUTS\n");
      send_msg=devfs_puts(vfs_msg);
      serial_print("PUTS DONE\n");
      break;
      case VFS_GETS:
      serial_print("GETS\n");
      send_msg=devfs_gets(vfs_msg);
      serial_print("GETS DONE\n");
      break;
      case VFS_MOUNT:
      serial_print("MOUNT\n");
      send_msg=devfs_mount(vfs_msg);
      serial_print("MOUNT DONE\n");
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
      free(msg.msg);
    }
    mailbox_get_msg(devfs_drv_box,&msg,sizeof(vfs_message));
    if (msg.from==0) {
      free(msg.msg);
    } else {
      vfs_message* vfs_msg=(vfs_message*)msg.msg;
      process_vfs_msg(vfs_msg,msg.from);
      free(msg.msg);
    }
    msg.msg=malloc(sizeof(devfs_message));
    mailbox_get_msg(devfs_reg_box,&msg,sizeof(devfs_message));
    if (msg.from==0) {
      free(msg.msg);
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
      free(msg.msg);
    }
    yield();
  }
}
