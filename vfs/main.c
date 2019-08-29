#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>
#include <stdlib.h>
#include <string.h>
#include <dbg.h>

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
static uint32_t num_drvs;
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
  drv_names=malloc(sizeof(const char**)*32);
  drvs=malloc(sizeof(uint32_t)*32);
  max_drvs=32;
  num_drvs=0;
  head_mapping=NULL;
  tail_mapping=NULL;
}

uint32_t register_fs_intern(uint32_t drv,const char* type) {
  if (num_drvs==max_drvs) {
    drvs=realloc(drvs,sizeof(uint32_t)*(max_drvs+32));
    drv_names=realloc(drv_names,sizeof(char*)*(max_drvs+32));
    max_drvs+=32;
  }
  drvs[num_drvs]=drv;
  drv_names[num_drvs]=type;
  num_drvs++;
  return num_drvs-1;
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
        vfs_msg->flags=2;
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
  vfs_msg->flags=2;
}

void vfs_putc(vfs_message* vfs_msg,uint32_t from) {
  uint32_t fd=vfs_msg->fd;
  vfs_file file_info=fd_tables[from][fd];
  strcpy(&vfs_msg->path[0],file_info.path);
  strcpy(&vfs_msg->mode[0],file_info.mode);
  vfs_msg->pos=file_info.pos;
  Message msg;
  msg.from=box;
  msg.to=drvs[file_info.mntpnt->type];
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

void vfs_register_fs(vfs_message* vfs_msg, uint32_t from) {
  char* name=malloc(sizeof(char)*(strlen(vfs_msg->mode)+1));
  name[0]='\0';
  strcpy(name,&vfs_msg->mode[0]);
  register_fs_intern(vfs_msg->fd,name);
  vfs_msg->flags=0;
}

void vfs_mount(vfs_message* vfs_msg, uint32_t from) {
  char* path=malloc(sizeof(char)*(strlen(vfs_msg->path)+1));
  path[0]='\0';
  strcpy(path,&vfs_msg->path[0]);
  char* type=malloc(sizeof(char)*(strlen(vfs_msg->mode)+1));
  type[0]='\0';
  strcpy(type,&vfs_msg->mode[0]);
  char* disk_file=malloc(sizeof(char)*(vfs_msg->data));
  Message msg;
  msg.msg=disk_file;
  mailbox_get_msg(box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  if (msg.from==0) {
    vfs_msg->flags=2;
    return;
  }
  char found=0;
  uint32_t i;
  for (i=0;i<num_drvs;i++) {
    if (strcmp(type,drv_names[i])==0) {
      found=1;
      break;
    }
  }
  if (!found) {
    vfs_msg->flags=2;
    return;
  }
  if (head_mapping==NULL) {
    vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
    mapping->mntpnt=malloc(sizeof(char)*(strlen(path)+1));
    strcpy(mapping->mntpnt,path);
    mapping->type=i;
    mapping->next=NULL;
    head_mapping=mapping;
    tail_mapping=mapping;
  } else {
    vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
    mapping->mntpnt=malloc(sizeof(char)*(strlen(path)+1));
    strcpy(mapping->mntpnt,path);
    mapping->type=i;
    mapping->next=NULL;
    tail_mapping->next=mapping;
  }
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
      case VFS_MOUNT:
      vfs_mount(vfs_msg,msg.from);
      break;
      case VFS_REGISTER_FS:
      vfs_register_fs(vfs_msg,msg.from);
      break;
      default:
      vfs_msg->flags=1;
      break;
    }
    msg.from=box;
    msg.to=sender;
    mailbox_send_msg(&msg);
    yield();
  }
  for (;;);
}
