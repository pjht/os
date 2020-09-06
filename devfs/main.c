#include <rpc.h>
#include <pthread.h>
#include <serdes.h>
#include <dbg.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int num_devs;
int max_devs;
char** dev_names;
pid_t* dev_pids;

void devfs_mount(void* args) {
  serdes_state state;
  serialize_int(0,&state);
  serialize_ptr(NULL,&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

void open(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  char* path=deserialize_str(&state);
  deserialize_ptr(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  int i;
  char found=0;
  for (i=0;i<num_devs;i++) {
    if (strcmp(dev_names[i],path)==0) {
      found=1;
      break;
    }
  }
  if (!found) {
    state.buf=NULL;
    state.sizeorpos=0;
    serialize_int(1,&state);
    serialize_ptr(NULL,&state);
    serialize_int(0,&state);
    rpc_return(state.buf,state.sizeorpos);
    pthread_exit(NULL);
  }
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_int(0,&state);
  serialize_ptr(NULL,&state);
  serialize_int(dev_pids[i],&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

void register_dev(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  char* name=deserialize_str(&state);
  pid_t pid=deserialize_int(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  if (num_devs==max_devs) {
    max_devs+=32;
    dev_names=realloc(dev_names,sizeof(char*)*max_devs);
    dev_pids=realloc(dev_pids,sizeof(pid_t)*max_devs);
  }
  dev_names[num_devs]=name;
  dev_pids[num_devs]=pid;
  num_devs++;
  rpc_return(NULL,0);
  pthread_exit(NULL);
}

int main() {
  num_devs=0;
  max_devs=32;
  dev_names=malloc(sizeof(char*)*32);
  dev_pids=malloc(sizeof(pid_t)*32);
  rpc_register_func("mount",&devfs_mount);
  rpc_register_func("open",&open);
  rpc_register_func("register_dev",&register_dev);
  register_fs("devfs",getpid());
  serial_print("Devfs initialized\n");
  rpc_mark_as_init();
}

