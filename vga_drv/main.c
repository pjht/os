#include <rpc.h>
#include <serdes.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <dbg.h>
#include "vga.h"

void write(void* args) {
  serdes_state state;
  start_deserialize(args,&state);
  deserialize_ptr(&state);
  size_t size=deserialize_int(&state);
  deserialize_int(&state);
  char* buf=deserialize_ary(size,&state);
  rpc_deallocate_buf(args,state.sizeorpos);
  vga_write_string(buf);
  state.buf=NULL;
  state.sizeorpos=0;
  serialize_int(size,&state);
  rpc_return(state.buf,state.sizeorpos);
  free(state.buf);
  pthread_exit(NULL);
}

int main() {
  vga_init();
  rpc_register_func("write",&write);
  serdes_state state={0};
  serialize_str("vga",&state);
  serialize_int(getpid(),&state);
  rpc_call(3,"register_dev",state.buf,state.sizeorpos);
  free(state.buf);
  vga_write_string("[INFO] VGA driver initialized\n");
  rpc_mark_as_init();
}
