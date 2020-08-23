#include <rpc.h>
#include <pthread.h>
#include <serdes.h>
#include <string.h>
#include <dbg.h>
#include <stdlib.h>

typedef struct mount_point {
  char* path;
  pid_t fs_pid;
  void* fs_data;
  struct mount_point* next;
} mount_point;

typedef struct fs_type {
  char* name;
  pid_t fs_pid;
  struct fs_type* next; 
} fs_type;

static int vfsstrcmp(const char* s1,const char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    if (s1[i] == '\0') return 0;
    return s1[i] - s2[i];
}

mount_point* mount_point_list=NULL;
fs_type* fs_type_list=NULL;

void vfs_mount(char* args) {
  serdes_state state;
  start_deserialize(args,&state);
  char* type=deserialize_str(&state);
  char* dev=deserialize_str(&state);
  char* mount_path=deserialize_str(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  fs_type* fstype=fs_type_list;
  pid_t fs_pid=0;
  size_t mntpnt_len=0;
  for (;fstype!=NULL;fstype=fstype->next) {
    if (vfsstrcmp(type,fstype->name)==0) {
      fs_pid=fstype->fs_pid;
      break;
    }
  }
  if (!fs_pid) {
    int err=1;
    rpc_return(&err,sizeof(int));
    pthread_exit(NULL);
  }
  char* retbuf=rpc_call(fs_pid,"mount",dev,strlen(dev)+1);
  start_deserialize(retbuf,&state);
  int err=deserialize_int(&state);
  void* data=deserialize_ptr(&state);
  rpc_deallocate_buf(retbuf, state.sizeorpos);
  int* errbuf=malloc(sizeof(int));
  *errbuf=err;
  if (err) {
    rpc_return(errbuf,sizeof(int));
    free(errbuf);
    pthread_exit(NULL);
  } 
  mount_point* mnt_pnt=malloc(sizeof(mnt_pnt));
  mnt_pnt->fs_data=data;
  mnt_pnt->fs_pid=fs_pid;
  mnt_pnt->path=mount_path;
  mnt_pnt->next=mount_point_list;
  mount_point_list=mnt_pnt;
  rpc_return(errbuf,sizeof(int));
  free(errbuf);
  pthread_exit(NULL);
}

void vfs_register_fs(char* args) {
  serdes_state state;
  start_deserialize(args,&state);
  char* name=deserialize_str(&state);
  pid_t pid=deserialize_int(&state);
  fs_type* type=malloc(sizeof(fs_type));
  rpc_deallocate_buf(args,state.sizeorpos);
  type->name=name;
  type->fs_pid=pid;
  type->next=fs_type_list;
  fs_type_list=type;
  rpc_return(NULL,0);
  pthread_exit(NULL);
}

void open(char* args) {
  serdes_state state;
  start_deserialize(args,&state);
  pid_t pid=deserialize_int(&state);
  int perms=deserialize_int(&state);
  char* path=deserialize_str(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  mount_point* mnt=mount_point_list;
  mount_point* mnt_pnt=NULL;
  size_t mntpnt_len=0;
  for (;mnt!=NULL;mnt=mnt->next) {
    char* root=mnt->path;
    if (strlen(root)>mntpnt_len) {
      if (vfsstrcmp(root,path)==0) {
        mnt_pnt=mnt;
        mntpnt_len=strlen(root);
      }
    }
  }
  if (mnt_pnt==NULL) {
    serial_print("NO MOUNTPOINT\n");
    state.buf=NULL;
    state.sizeorpos=0;
    serialize_int(1,&state);
    serialize_ptr(NULL,&state);
    serialize_int(0,&state);
    rpc_return(state.buf,state.sizeorpos);
    free(state.buf);
    pthread_exit(NULL);
  }
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_str(path+(strlen(mnt_pnt->path)+1),&state);
  char* retbuf=rpc_call(mnt_pnt->fs_pid,"open",state.buf,state.sizeorpos);
  free(state.buf);
  start_deserialize(retbuf,&state);
  int err=deserialize_int(&state);
  void* data=deserialize_ptr(&state);
  pid_t alt_pid=deserialize_int(&state);
  pid_t fs_pid = alt_pid ? alt_pid : mnt_pnt->fs_pid;
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_int(err,&state);
  serialize_ptr(data,&state);
  serialize_int(fs_pid,&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

int main() {
  rpc_register_func("mount",&vfs_mount);
  rpc_register_func("open",&open);
  rpc_register_func("register_fs",&vfs_register_fs);
  rpc_mark_as_init();
}
