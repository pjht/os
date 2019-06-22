#include <tasking.h>
#include <ipc/vfs.h>

int main() {
  int sender;
  int size;
  vfs_message* msg=get_msg(&sender,&size);
  msg->fd=2;
  send_msg(1,msg,size);
  yield();
  for (;;);
}
