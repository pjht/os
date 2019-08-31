#include <mailboxes.h>
#include <string.h>
#include <stdlib.h>
#include <ipc/vfs.h>
#include <stdio.h>
#include <tasking.h>
#include <dbg.h>
#define VFS_MBOX 3
#define VFS_PID 2


static uint32_t box;
FILE* __stdio_stdin;
FILE* __stdio_stdout;
FILE* __stdio_stderr;


void __stdio_init() {
  box=mailbox_new(16);
  __stdio_stdin=malloc(sizeof(FILE*));
  *__stdio_stdin=0;
  __stdio_stdout=malloc(sizeof(FILE*));
  *__stdio_stdout=1;
  __stdio_stderr=malloc(sizeof(FILE*));
  *__stdio_stderr=2;
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

int putc(int c, FILE* stream) __attribute__ ((alias ("fputc")));

int fputc(int c, FILE* stream) {
  char str[]={c,'\0'};
  if (fputs(str,stream)==0) {
    return c;
  } else {
    return EOF;
  }
  return EOF;
}

int fputs(const char* s, FILE* stream) {
  vfs_message* msg_data=make_msg(VFS_PUTS,NULL,NULL,*stream,strlen(s));
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=(char*)s;
  msg.size=strlen(s);
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
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
    return 0;
  }
  return EOF;
}

size_t fwrite(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  char* buffer=(char*)buffer_ptr;
  size_t bytes=size*count;
  vfs_message* msg_data=make_msg(VFS_PUTS,NULL,NULL,*stream,bytes);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=buffer;
  msg.size=bytes;
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return 0;
  } else {
    free(msg.msg);
    return count;
  }
  return 0;
}

void register_fs(const char* name,uint32_t mbox) {
  vfs_message* msg_data=make_msg(VFS_REGISTER_FS,name,NULL,mbox,0);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yieldToPID(VFS_PID);
  msg.msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  free(msg.msg);
}

void mount(char* file,char* type,char* path) {
  vfs_message* msg_data=make_msg(VFS_MOUNT,type,path,0,strlen(file)+1);
  Message msg;
  msg.from=box;
  msg.to=VFS_MBOX;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=file;
  msg.size=strlen(file)+1;
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  free(msg.msg);
}
