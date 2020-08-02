#include <dbg.h>
#include <rpc.h>
#include <tasking.h>
#include <pthread.h>
#include <string.h>

void rpcfunc(void* buf) {
  if(buf) {
    serial_print(buf);
  } else {
   serial_print("RPC test func\n");
  }
  rpc_return("return value\n",strlen("return value\n")+1);
  pthread_exit(NULL);
}

int main() {
  rpc_register_func("rpctestfunc",&rpcfunc);
  rpc_mark_as_init();
}
