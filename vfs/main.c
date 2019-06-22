#include <tasking.h>

int main() {
  int sender=&sender;
  char* msg=get_msg(&sender);
  send_msg(1,msg,3);
  yield();
  for (;;);
}
