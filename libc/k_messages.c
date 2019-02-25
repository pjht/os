#include <string.h>
#include <tasking.h>
#include <stdint.h>

void* get_paddr(void* addr_arg) {
  uint32_t addr=(uint32_t)addr_arg;
  char msg[15];
  strcpy(msg,"GET_PADDR ");
  msg[14]=0;
  msg[10]=addr&0xFF;
  msg[11]=(addr&0xFF00)>>8;
  msg[12]=(addr&0xFF0000)>>16;
  msg[13]=(addr&0xFF000000)>>24;
  send_msg(0,msg);
  char* recv_msg=NULL;
  while (recv_msg==NULL) {
    uint32_t sender;
    recv_msg=get_msg(&sender);
    yield();
  }
  uint32_t recv_addr=recv_msg[0];
  recv_addr=recv_addr|recv_msg[1]<<8;
  recv_addr=recv_addr|recv_msg[2]<<16;
  recv_addr=recv_addr|recv_msg[3]<<24;
  return (void*)recv_addr;
}
