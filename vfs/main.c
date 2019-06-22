#include <tasking.h>

int main() {
  int sender;
  int size;
  char* msg=get_msg(&sender,&size);
  send_msg(1,msg,size);
  yield();
  for (;;);
}
