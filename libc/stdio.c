#include <mailboxes.h>
#include <string.h>
#include <stdlib.h>
#include <ipc/vfs.h>
#include <stdio.h>
#include <tasking.h>
#define VFS_MBOX 3
#define VFS_PID 2


static uint32_t box;

void __stdio_init() {
  box=mailbox_new(16);
}

static vfs_message* make_msg(vfs_message_type type,const char* mode,const char* path, uint32_t fd, int data) {
  static uint32_t id=0;
  vfs_message* msg_data=malloc(sizeof(vfs_message));
  msg_data->type=type;
  msg_data->id=id;
  msg_data->fd=fd;
  msg_data->data=data;
  id++;
  if (mode!=NULL) {
    strcpy(&msg_data->mode[0],mode);
  }
  if (path!=NULL) {
    strcpy(&msg_data->path[0],path);
  }
  return msg_data;
}

FILE* fopen(char* filename,char* mode) {
  if (strlen(filename)>4096 || strlen(mode)>10) {
    return NULL;
  }
  vfs_message* msg_data=make_msg(VFS_OPEN,mode,filename,0,0);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yieldToPID(VFS_PID);
  msg.msg=malloc(sizeof(vfs_message));
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
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

int fputc(int c, FILE* stream) {
  vfs_message* msg_data=make_msg(VFS_PUTC,0,0,*stream,c);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yieldToPID(VFS_PID);
  msg.msg=malloc(sizeof(vfs_message));
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return EOF;
  } else {
    free(msg.msg);
    return c;
  }
}

int fputs(const char* s, FILE* stream) {
  for (int i=0;s[i]!='\0';i++) {
    if (fputc(s[i],stream)==EOF) {
      return EOF;
    };
  }
  return 0;
}

void register_fs(const char* name) {
  vfs_message* msg_data=make_msg(VFS_REGISTER_FS,NULL,name,0,0);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yieldToPID(VFS_PID);
  msg.msg=malloc(sizeof(vfs_message));
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  free(msg.msg);
}
