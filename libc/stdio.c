#include <mailboxes.h>
#include <string.h>
#include <stdlib.h>
#include <ipc/vfs.h>
#include <stdio.h>
#include <tasking.h>
#define VFS_PID 1


static uint32_t box;

void __stdio_init() {
  box=mailbox_new(16);
}

static vfs_message* make_msg(vfs_message_type type, char* mode, char* path) {
  static uint32_t id=0;
  vfs_message* msg_data=malloc(sizeof(vfs_message));
  msg_data->type=type;
  msg_data->id=id;
  id++;
  strcpy(&msg_data->mode[0],mode);
  strcpy(&msg_data->path[0],path);
  return msg_data;
}

FILE* fopen(char* filename,char* mode) {
  if (strlen(filename)>4096 || strlen(mode)>10) {
    return NULL;
  }
  vfs_message* msg_data=make_msg(VFS_OPEN,mode,filename);
  Message msg;
  msg.from=box;
  msg.to=VFS_PID;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yield();
  msg.msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yield();
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return NULL;
  } else {
    FILE* file=malloc(sizeof(FILE));
    *file=vfs_msg->fd; //We're using pointers to FILE even though it's a uint32_t so we can expand to a struct if needed
    free(msg.msg);
    return file;
  }
}
