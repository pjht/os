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
  char* mntpnt;
  char* path;
  uint32_t pos;
  char error;
} vfs_file;

static const char** drv_names;
static uint32_t* drvs;
static uint32_t max_drvs;
static uint32_t next_drv_indx;
static vfs_mapping* head_mapping;
static vfs_mapping* tail_mapping;
uint32_t* fd_tables[32768];
uint16_t open_fds[32768];

vfs_message* get_message(Message* msg,uint32_t box) {
  msg->msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,msg,sizeof(vfs_message));
  while (msg->from==0 && msg->size==0) {
    mailbox_get_msg(box,msg,sizeof(vfs_message));
    yield();
  }
  vfs_message* vfs_msg=(vfs_message*)msg->msg;
  msg->to=msg->from;
  msg->from=box;
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
  drvs[0]=2;
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

char vfs_fopen(vfs_message* vfs_msg,uint32_t from) {
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
  if (mntpnt) {
    Message msg;
    char* path_buf=malloc(sizeof(char)*4096);
    strcpy(path_buf,&(vfs_msg->path[0]));
    memset(&(vfs_msg->path[0]),0,sizeof(char)*4096);
    for (size_t i=0;i<strlen(path_buf)+1-mntpnt_len;i++) {
      vfs_msg->path[i]=path_buf[i+mntpnt_len];
    }
    free(path_buf);
    msg.from=1;
    msg.to=drvs[mntpnt->type];
    msg.size=sizeof(vfs_message);
    msg.msg=vfs_msg;
    mailbox_send_msg(&msg);
    yield();
    vfs_message* vfs_msg=get_message(&msg,1);
    if (vfs_msg->flags!=0) {
      return vfs_msg->flags;
    }
    if (fd_tables[from]==NULL) {
      fd_tables[from]=malloc(PROC_FD_LIMIT*sizeof(vfs_file));
      open_fds[from]=1;
    } else {
      if (open_fds[from]==PROC_FD_LIMIT) {
        return 4;
      }
    }
    uint16_t fd=open_fds[from];
    open_fds[from]++;
    vfs_msg->fd=fd;
    return 0;
  }
  return 1;
}

int main() {
  init_vfs();
  uint32_t box=mailbox_new(16);
  yield();
  while (1) {
    Message msg;
    vfs_message* vfs_msg=get_message(&msg,box);
    switch (vfs_msg->type) {
      case VFS_OPEN:
      vfs_msg->flags=vfs_fopen(vfs_msg,msg.from);
      break;
      case VFS_PUTC:
      vfs_msg->flags=1;
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
    mailbox_send_msg(&msg);
    yield();
  }
  for (;;);
}
