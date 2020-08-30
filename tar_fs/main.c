#include <stddef.h>
#include <string.h>
#include <serdes.h>
#include <rpc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbg.h>

typedef struct {
  char filename[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
} tar_header;

typedef struct tar_file {
  int pos;
  char name[100];
  char* dev;
  struct tar_file* next;
} tar_file;

typedef struct {
  FILE* access_file;
  int base_pos;
} open_file_info;

size_t getsize(const char *in) {
    size_t size=0;
    size_t j;
    size_t count=1;
    for (j=11;j>0;j--,count*=8) {
        size+=((in[j-1]-'0')*count);
    }
    return size;
}

tar_file* get_file_list(char* dev) {
  FILE* tar_archive=fopen(dev,"r");
  size_t pos=0;
  tar_header tar_hdr;
  tar_file* list=NULL;
  for (int i=0;;i++) {
    fseek(tar_archive,pos,SEEK_SET);
    fread(&tar_hdr,sizeof(tar_header),1,tar_archive);
    if (tar_hdr.filename[0]=='\0') break;
    size_t size=getsize(tar_hdr.size);
    pos+=512;
    tar_file* list_entry=malloc(sizeof(tar_file));
    list_entry->pos=pos;
    list_entry->dev=dev;
    strcpy(list_entry->name,tar_hdr.filename);
    list_entry->next=list;
    list=list_entry;
    pos+=size;
    if (pos%512!=0) {
      pos+=512-(pos%512);
    }
  }
  return list;
}

void tar_fs_mount(void* args) {
  char* dev=(char*)args;
  tar_file* file_list=get_file_list(dev);
  serdes_state state;
  serialize_int(0,&state);
  serialize_ptr(file_list,&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

void open(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  char* path=deserialize_str(&state);
  tar_file* file_list=deserialize_ptr(&state);
  for (;file_list!=NULL;file_list=file_list->next) {
    if (strcmp(path,file_list->name)==0) {
      break;
    }
  }
  if (file_list) {
    open_file_info* info=malloc(sizeof(open_file_info));
    info->access_file=fopen(file_list->dev,"r");
    info->base_pos=file_list->pos;
    state.buf=NULL;
    state.sizeorpos=0;
    serialize_int(0,&state);
    serialize_ptr(info,&state);
    serialize_int(0,&state);
    rpc_return(state.buf,state.sizeorpos);
    pthread_exit(NULL);
  } else {
    state.buf=NULL;
    state.sizeorpos=0;
    serialize_int(1,&state);
    serialize_ptr(NULL,&state);
    serialize_int(0,&state);
    rpc_return(state.buf,state.sizeorpos);
    pthread_exit(NULL);
  }
}

void read(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  open_file_info* info=deserialize_ptr(&state);
  size_t size=deserialize_int(&state);
  int pos=deserialize_int(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  char* data=malloc(sizeof(char)*size);
  fseek(info->access_file,info->base_pos+pos,SEEK_SET);
  fread(data,size,1,info->access_file);
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_int(size,&state);
  serialize_ary(data,size,&state);
  free(data);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

int main() {
  rpc_register_func("mount",&tar_fs_mount);
  rpc_register_func("open",&open);
  rpc_register_func("read",&read);
  serial_print("Registering tarfs filesystem\n");
  register_fs("tarfs",getpid());
  serial_print("Initialized tarfs\n");
  rpc_mark_as_init();
}
