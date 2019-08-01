#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>
#include <stdlib.h>
#include <string.h>

#define PROC_FD_LIMIT 1024

typedef struct _vfs_mapping_struct {
  char* mntpnt;
  uint32_t type;
  struct _vfs_mapping_struct* next;
} vfs_mapping;

typedef struct {
  vfs_mapping* mntpnt;
  char* path;
  char* mode;
  uint32_t pos;
  char error;
} vfs_file;

static const char** drv_names;
static uint32_t* drvs;
static uint32_t max_drvs;
static uint32_t next_drv_indx;
static vfs_mapping* head_mapping;
static vfs_mapping* tail_mapping;
vfs_file* fd_tables[32768];
uint16_t open_fds[32768];
uint32_t box;
vfs_message* get_message(Message* msg) {
  msg->msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,msg,sizeof(vfs_message));
  while (msg->from==0 && msg->size==0) {
    yield();
    mailbox_get_msg(box,msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg->msg;
  return vfs_msg;
}

static int vfsstrcmp(const char* s1,const char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    if (s1[i] == '\0') return 0;
    return s1[i] - s2[i];
}

void init_vfs() {
  drvs=malloc(sizeof(uint32_t)*32);
  drvs[0]=5;
  drv_names=malloc(sizeof(const char**)*32);
  max_drvs=32;
  next_drv_indx=0;
  head_mapping=malloc(sizeof(vfs_mapping));
  head_mapping->mntpnt="/dev/";
  head_mapping->type=0;
  head_mapping->next=NULL;
  tail_mapping=NULL;
}

uint32_t register_fs(uint32_t drv,const char* type) {
  if (next_drv_indx==max_drvs) {
    drvs=realloc(drvs,sizeof(uint32_t)*(max_drvs+32));
    drv_names=realloc(drv_names,sizeof(char*)*(max_drvs+32));
    max_drvs+=32;
  }
  drvs[next_drv_indx]=drv;
  drv_names[next_drv_indx]=type;
  next_drv_indx++;
  return next_drv_indx-1;
}

void vfs_fopen(vfs_message* vfs_msg,uint32_t from) {
  vfs_mapping* mnt=head_mapping;
  vfs_mapping* mntpnt=NULL;
  uint32_t mntpnt_len=0;
  for (;mnt!=NULL;mnt=mnt->next) {
    char* root=mnt->mntpnt;
    if (strlen(root)>mntpnt_len) {
      if (vfsstrcmp(root,vfs_msg->path)==0) {
        mntpnt=mnt;
        mntpnt_len=strlen(root);
      }
    }
  }
  if (mntpnt) { // was if (mntpnt)
    Message msg;
    char* path_buf=malloc(sizeof(char)*4096);
    strcpy(path_buf,&(vfs_msg->path[0]));
    memset(&(vfs_msg->path[0]),0,sizeof(char)*4096);
    for (size_t i=0;i<strlen(path_buf)+1-mntpnt_len;i++) {
      vfs_msg->path[i]=path_buf[i+mntpnt_len];
    }
    free(path_buf);
    msg.from=box;
    msg.to=drvs[mntpnt->type];
    msg.size=sizeof(vfs_message);
    msg.msg=vfs_msg;
    mailbox_send_msg(&msg);
    yield();
    vfs_message* resp_msg=get_message(&msg);
    if (resp_msg->flags!=0) {
      return;
    }
    if (fd_tables[from]==NULL) {
      fd_tables[from]=malloc(PROC_FD_LIMIT*sizeof(vfs_file));
      open_fds[from]=0;
    } else {
      if (open_fds[from]==PROC_FD_LIMIT) {
        vfs_msg->flags=4;
      }
    }
    uint16_t fd=open_fds[from];
    open_fds[from]++;
    fd_tables[from][fd].mntpnt=mntpnt;
    fd_tables[from][fd].path=malloc(sizeof(char)*(strlen(&vfs_msg->path[0])+1));
    strcpy(fd_tables[from][fd].path,&vfs_msg->path[0]);
    fd_tables[from][fd].mode=malloc(sizeof(char)*(strlen(&vfs_msg->mode[0])+1));
    strcpy(fd_tables[from][fd].mode,&vfs_msg->mode[0]);
    fd_tables[from][fd].pos=0;
    fd_tables[from][fd].error=0;
    vfs_msg->fd=fd;
    vfs_msg->flags=0;
    return;
  }
  vfs_msg->flags=1;
}

void vfs_putc(vfs_message* vfs_msg,uint32_t from) {
  uint32_t fd=vfs_msg->fd;
  vfs_file file_info=fd_tables[from][fd];
  strcpy(&vfs_msg->path[0],file_info.path);
  strcpy(&vfs_msg->mode[0],file_info.mode);
  vfs_msg->pos=file_info.pos;
  Message msg;
  msg.from=box;
  msg.to=file_info.mntpnt->type;
  msg.to=5;
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  yield();
  vfs_msg=get_message(&msg);
  if (vfs_msg->flags!=0) {
    return;
  }
  fd_tables[from][fd].pos++;
  vfs_msg->flags=0;
}

int main() {
  init_vfs();
  box=mailbox_new(16);
  yield();
  while (1) {
    Message msg;
    vfs_message* vfs_msg=get_message(&msg);
    uint32_t sender=msg.from;
    switch (vfs_msg->type) {
      case VFS_OPEN:
      vfs_fopen(vfs_msg,msg.from);
      break;
      case VFS_PUTC:
      vfs_putc(vfs_msg,msg.from);
      break;
      case VFS_GETC:
      vfs_msg->flags=1;
      break;
      case VFS_CLOSE:
      vfs_msg->flags=1;
      break;
      case VFS_MOUNT:
      vfs_msg->flags=1;
      break;
      case VFS_UMOUNT:
      vfs_msg->flags=1;
      break;
      default:
      vfs_msg->flags=2;
      break;
    }
    msg.from=box;
    msg.to=sender;
    mailbox_send_msg(&msg);
    yield();
  }
  for (;;);
}
