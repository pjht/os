#include <tasking.h>
#include <ipc/vfs.h>
#include <mailboxes.h>
#include <stdlib.h>

typedef struct _vfs_mapping_struct {
  char* mntpnt;
  uint32_t type;
  struct _vfs_mapping_struct* next;
} vfs_mapping;

static const char** drv_names;
static uint32_t* drvs;
static uint32_t max_drvs;
static uint32_t next_drv_indx;
static vfs_mapping* head_mapping;
static vfs_mapping* tail_mapping;

vfs_message* get_message(Message* msg,uint32_t box) {
  msg->msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,msg,sizeof(vfs_message));
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
  drv_names=malloc(sizeof(const char**)*32);
  max_drvs=32;
  next_drv_indx=0;
  head_mapping=NULL;
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

int main() {
  init_vfs();
  uint32_t box=mailbox_new(16);
  yield();
  Message msg;
  vfs_message* vfs_msg=get_message(&msg,box);
  switch (vfs_msg->type) {
    case VFS_OPEN:
    vfs_msg->flags=1;
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
  for (;;);
}
