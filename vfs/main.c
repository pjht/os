#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>
#include <stdlib.h>
#include <string.h>
#include <dbg.h>

#define PROC_FD_LIMIT 1024
#define ID_LIMIT 256

typedef struct _vfs_mapping_struct {
  char* mntpnt;
  size_t type;
  struct _vfs_mapping_struct* next;
} vfs_mapping;

typedef struct {
  vfs_mapping* mntpnt;
  char* path;
  char* mode;
  size_t pos;
  char error;
  void* fs_data;
} vfs_file;

typedef struct {
  int fd;
  int max_len;
} gets_data;


typedef struct {
  char* path;
  size_t type;
} mount_data;

static const char** drv_names;
static uint32_t* drvs;
static size_t max_drvs;
static size_t num_drvs;
static vfs_mapping* head_mapping;
static vfs_mapping* tail_mapping;
vfs_file* fd_tables[32768];
int open_fds[32768];
uint32_t box;
void** in_progress_data[32768];
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

size_t register_fs_intern(uint32_t drv,const char* type) {
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
  if (fd_tables[from]!=NULL&&open_fds[from]==PROC_FD_LIMIT) {
    vfs_msg->flags=2;
  }
  vfs_mapping* mnt=head_mapping;
  vfs_mapping* mntpnt=NULL;
  size_t mntpnt_len=0;
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
    msg.from=box;
    msg.to=drvs[mntpnt->type];
    msg.size=sizeof(vfs_message);
    msg.msg=vfs_msg;
    mailbox_send_msg(&msg);
    in_progress_data[from][vfs_msg->id]=mntpnt;
  } else {
    vfs_msg->flags=2;
  }
  vfs_msg->flags=0;
}
void vfs_fopen_finish(vfs_message* vfs_msg,uint32_t from) {
  vfs_mapping* mntpnt=in_progress_data[vfs_msg->orig_mbox][vfs_msg->id];
  if (fd_tables[vfs_msg->orig_mbox]==NULL) {
    fd_tables[vfs_msg->orig_mbox]=malloc(PROC_FD_LIMIT*sizeof(vfs_file));
    open_fds[vfs_msg->orig_mbox]=0;
  }
  int fd=open_fds[vfs_msg->orig_mbox];
  open_fds[vfs_msg->orig_mbox]++;
  fd_tables[vfs_msg->orig_mbox][fd].mntpnt=mntpnt;
  fd_tables[vfs_msg->orig_mbox][fd].path=malloc(sizeof(char)*(strlen(&vfs_msg->path[0])+1));
  strcpy(fd_tables[vfs_msg->orig_mbox][fd].path,&vfs_msg->path[0]);
  fd_tables[vfs_msg->orig_mbox][fd].mode=malloc(sizeof(char)*(strlen(&vfs_msg->mode[0])+1));
  strcpy(fd_tables[vfs_msg->orig_mbox][fd].mode,&vfs_msg->mode[0]);
  fd_tables[vfs_msg->orig_mbox][fd].pos=0;
  fd_tables[vfs_msg->orig_mbox][fd].error=0;
  fd_tables[vfs_msg->orig_mbox][fd].fs_data=vfs_msg->fs_data;
  vfs_msg->fd=fd;
  vfs_msg->flags=0;
}

void vfs_fopen_abort(vfs_message* vfs_msg,uint32_t from) {
}

void vfs_puts(vfs_message* vfs_msg,uint32_t from) {
  char* data=malloc(sizeof(char)*vfs_msg->data);
  Message msg;
  msg.msg=data;
  mailbox_get_msg(box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  if (msg.from==0) {
    vfs_msg->flags=2;
    return;
  }
  int fd=vfs_msg->fd;
  vfs_file file_info=fd_tables[from][fd];
  strcpy(&vfs_msg->path[0],file_info.path);
  strcpy(&vfs_msg->mode[0],file_info.mode);
  vfs_msg->pos=file_info.pos;
  vfs_msg->fs_data=fd_tables[from][fd].fs_data;
  msg.from=box;
  msg.to=drvs[file_info.mntpnt->type];
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  msg.size=vfs_msg->data;
  msg.msg=data;
  mailbox_send_msg(&msg);
  free(data);
  in_progress_data[from][vfs_msg->id]=malloc(sizeof(int));
  *((int*)in_progress_data[from][vfs_msg->id])=fd;
  vfs_msg->flags=0;
}

void vfs_puts_finish(vfs_message* vfs_msg,uint32_t from) {
  int fd=*((int*)in_progress_data[vfs_msg->orig_mbox][vfs_msg->id]);
  free(in_progress_data[vfs_msg->orig_mbox][vfs_msg->id]);
  fd_tables[vfs_msg->orig_mbox][fd].pos+=vfs_msg->data;
  vfs_msg->flags=0;
}

void vfs_puts_abort(vfs_message* vfs_msg,uint32_t from) {
  free(in_progress_data[vfs_msg->orig_mbox][vfs_msg->id]);
}

void vfs_gets(vfs_message* vfs_msg,uint32_t from) {
  int fd=vfs_msg->fd;
  vfs_file file_info=fd_tables[from][fd];
  strcpy(&vfs_msg->path[0],file_info.path);
  strcpy(&vfs_msg->mode[0],file_info.mode);
  vfs_msg->pos=file_info.pos;
  vfs_msg->fs_data=fd_tables[from][fd].fs_data;
  Message msg;
  msg.from=box;
  msg.to=drvs[file_info.mntpnt->type];
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  gets_data* data=malloc(sizeof(gets_data));
  in_progress_data[from][vfs_msg->id]=data;
  data->fd=fd;
  data->max_len=vfs_msg->data;
  vfs_msg->flags=0;
}

char* vfs_gets_finish(vfs_message* vfs_msg,uint32_t from) {
  char* buf=malloc(sizeof(char)*vfs_msg->data);
  Message msg;
  msg.msg=buf;
  mailbox_get_msg(box,&msg,vfs_msg->data);
  while (msg.from==0 && msg.size==0) {
    yield();
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  if (msg.from==0) {
    vfs_msg->flags=2;
    return NULL;
  }
  gets_data* data=(gets_data*)in_progress_data[vfs_msg->orig_mbox][vfs_msg->id];
  if (vfs_msg->data>data->max_len) {
    vfs_msg->flags=2;
    return NULL;
  }
  vfs_msg->data=data->max_len;
  int fd=data->fd;
  free(in_progress_data[vfs_msg->orig_mbox][vfs_msg->id]);
  fd_tables[vfs_msg->orig_mbox][fd].pos+=vfs_msg->data;
  vfs_msg->flags=0;
  return buf;
}

void vfs_gets_abort(vfs_message* vfs_msg,uint32_t from) {
  free(in_progress_data[vfs_msg->orig_mbox][vfs_msg->id]);
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
    free(path);
    free(type);
    free(disk_file);
    vfs_msg->flags=2;
    return;
  }
  char found=0;
  size_t i;
  for (i=0;i<num_drvs;i++) {
    if (strcmp(type,drv_names[i])==0) {
      found=1;
      break;
    }
  }
  if (!found) {
    free(path);
    free(type);
    free(disk_file);
    vfs_msg->flags=2;
    return;
  }
  msg.from=box;
  msg.to=drvs[i];
  msg.size=sizeof(vfs_message);
  msg.msg=vfs_msg;
  mailbox_send_msg(&msg);
  msg.size=vfs_msg->data;
  msg.msg=disk_file;
  mailbox_send_msg(&msg);
  mount_data* data=malloc(sizeof(mount_data));
  in_progress_data[from][vfs_msg->id]=data;
  data->path=path;
  data->type=i;
  free(type);
  free(disk_file);
  vfs_msg->flags=0;
}

void vfs_mount_finish(vfs_message* vfs_msg,uint32_t from) {
  mount_data* data=in_progress_data[vfs_msg->orig_mbox][vfs_msg->id];
  if (head_mapping==NULL) {
    vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
    mapping->mntpnt=malloc(sizeof(char)*(strlen(data->path)+1));
    strcpy(mapping->mntpnt,data->path);
    mapping->type=data->type;
    mapping->next=NULL;
    head_mapping=mapping;
    tail_mapping=mapping;
  } else {
    vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
    mapping->mntpnt=malloc(sizeof(char)*(strlen(data->path)+1));
    strcpy(mapping->mntpnt,data->path);
    mapping->type=data->type;
    mapping->next=NULL;
    tail_mapping->next=mapping;
  }
  free(data->path);
  free(data);
  vfs_msg->flags=0;
}

void vfs_mount_abort(vfs_message* vfs_msg,uint32_t from)  {
  mount_data* data=in_progress_data[vfs_msg->orig_mbox][vfs_msg->id];
  free(data->path);
  free(data);
}

void vfs_seek(vfs_message* vfs_msg,uint32_t from) {
  char str[256];
  int_to_ascii(fd_tables[vfs_msg->orig_mbox][vfs_msg->fd].pos,str);
  serial_print("Prev pos:");
  serial_print(str);
  serial_print("\n");
  int_to_ascii(vfs_msg->pos,str);
  serial_print("New in vfs msg:");
  serial_print(str);
  serial_print("\n");
  fd_tables[vfs_msg->orig_mbox][vfs_msg->fd].pos=vfs_msg->pos;
  int_to_ascii(fd_tables[vfs_msg->orig_mbox][vfs_msg->fd].pos,str);
  serial_print("New in fd table:");
  serial_print(str);
  serial_print("\n");
  vfs_msg->flags=0;
}

int main() {
  init_vfs();
  box=mailbox_new(16,"vfs");
  yield();
  while (1) {
    Message msg;
    vfs_message* vfs_msg=get_message(&msg);
    uint32_t sender=msg.from;
    char* data=NULL;
    if (vfs_msg->in_progress) {
      if (vfs_msg->flags) {
        serial_print("ABORT\n");
        switch (vfs_msg->type) {
          case VFS_OPEN:
          vfs_fopen_abort(vfs_msg,msg.from);
          break;
          case VFS_PUTS:
          vfs_puts_abort(vfs_msg,msg.from);
          break;
          case VFS_GETS:
          vfs_gets_abort(vfs_msg,msg.from);
          break;
          case VFS_MOUNT:
          vfs_mount_abort(vfs_msg,msg.from);
          break;
          break;
          default:
          vfs_msg->flags=1;
          break;
        }
      } else {
        serial_print("FINISH TYPE ");
        char str[256];
        int_to_ascii(vfs_msg->type,str);
        serial_print(str);
        serial_print("\n");
        switch (vfs_msg->type) {
          case VFS_OPEN:
          serial_print("call fopen finish\n");
          vfs_fopen_finish(vfs_msg,msg.from);
          break;
          case VFS_PUTS:
          serial_print("call puts finish\n");
          vfs_puts_finish(vfs_msg,msg.from);
          break;
          case VFS_GETS:
          serial_print("call gets finish\n");
          data=vfs_gets_finish(vfs_msg,msg.from);
          break;
          case VFS_MOUNT:
          serial_print("call mount finish\n");
          vfs_mount_finish(vfs_msg,msg.from);
          break;
          default:
          serial_print("invalid finish\n");
          vfs_msg->flags=1;
          break;
        }
      }
      msg.from=box;
      msg.to=vfs_msg->orig_mbox;
      mailbox_send_msg(&msg);
      if (vfs_msg->type==VFS_GETS && data!=NULL) {
        msg.size=vfs_msg->data;
        msg.msg=data;
        mailbox_send_msg(&msg);
      }
    } else {
      vfs_msg->in_progress=1;
      if (in_progress_data[msg.from]==NULL) {
        in_progress_data[msg.from]=malloc(sizeof(void*)*ID_LIMIT);
      }
      serial_print("NEW\n");
      switch (vfs_msg->type) {
        case VFS_OPEN:
        vfs_fopen(vfs_msg,msg.from);
        break;
        case VFS_PUTS:
        vfs_puts(vfs_msg,msg.from);
        break;
        case VFS_GETS:
        vfs_gets(vfs_msg,msg.from);
        break;
        case VFS_MOUNT:
        vfs_mount(vfs_msg,msg.from);
        break;
        case VFS_REGISTER_FS:
        vfs_register_fs(vfs_msg,msg.from);
        serial_print("REGISTER_FS DONE\n");
        break;
        case VFS_SEEK:
        vfs_seek(vfs_msg,msg.from);
        break;
        default:
        vfs_msg->flags=1;
        break;
      }
      if (vfs_msg->flags || vfs_msg->type==VFS_REGISTER_FS || vfs_msg->type==VFS_SEEK) {
        msg.from=box;
        msg.to=sender;
        mailbox_send_msg(&msg);
      }
    }
    free(vfs_msg);
    yield();
  }
  for (;;);
}
