#include <dbg.h>
#include <initrd.h>
#include <rpc.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <serdes.h>
#include <unistd.h>

char* initrd;
long initrd_size;

void read(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  deserialize_ptr(&state);
  size_t size=deserialize_int(&state);
  int pos=deserialize_int(&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  long max_data=initrd_size-pos;
  if (size>max_data) {
    size=max_data;
  }
  char* data=malloc(sizeof(char)*size);
  for (long i=0;i<size;i++) {
    data[i]=initrd[i+pos]&0xFF;
    // serial_print("data[");
    // char str[256];
    // int_to_ascii(i,str);
    // serial_print(str);
    // serial_print("]=");
    // hex_to_ascii(data[i]&0xFF,str);
    // serial_print(str);
    // serial_print("\n");
  }
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_int(size,&state);
  serialize_ary(data,size,&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

int main() {
  initrd_size=initrd_sz();
  initrd=malloc(initrd_size);
  initrd_get(initrd);
  rpc_register_func("read",&read);
  serdes_state state={0};
  serialize_str("initrd",&state);
  serialize_int(getpid(),&state);
  rpc_call(3,"register_dev",state.buf,state.sizeorpos);
  free(state.buf);
  serial_print("Initrd driver initialized\n");
  rpc_mark_as_init();
}
