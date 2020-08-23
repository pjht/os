#include <dbg.h>
#include <initrd.h>
#include <rpc.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <serdes.h>

char* initrd;
long initrd_size;

void read(char* args) {
  serdes_state state;
  start_deserialize(args,&state);
  deserialize_ptr(&state);
  size_t size=deserialize_int(&state);
  int pos=deserialize_int(&state);
  rpc_deallocate_buf(args,state.buf);
  long max_data=initrd_size-pos;
  if (size>max_data) {
    serial_print("Reading too much data from initrd\n");
    state.buf=NULL;
    state.sizeorpos=0;
    serialize_int(0,&state);
    rpc_return(state.buf,state.sizeorpos);
    free(state.buf);
    pthread_exit(NULL);
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
  rpc_mark_as_init();
  serdes_state state={0};
  serialize_str("initrd",&state);
  serialize_int(getpid(),&state);
  rpc_call(3,"register_dev",state.buf,state.sizeorpos);
  free(state.buf);
  serial_print("Initrd driver initialized\n");
}

// #include <dbg.h>
// #include <initrd.h>
// #include <ipc/devfs.h>
// #include <ipc/vfs.h>
// #include <mailboxes.h>
// #include <memory.h>
// #include <stdlib.h>
// #include <string.h>
// #include <tasking.h>
// #include <vfs.h>

// #define VFS_PID 2
// #define DEVFS_PID 3

// uint32_t box;
// char* initrd;
// long size;

// void initrd_puts(vfs_message* vfs_msg) {
//   char* data=malloc(sizeof(char)*vfs_msg->data);
//   Message msg;
//   msg.msg=data;
//   mailbox_get_msg(box,&msg,vfs_msg->data);
//   while (msg.from==0 && msg.size==0) {
//     yield();
//     mailbox_get_msg(box,&msg,sizeof(vfs_message));
//   }
//   if (msg.from==0) {
//     serial_print("Could not recieve fputs data from the VFS\n");
//     vfs_msg->flags=2;
//     return;
//   }
//   free(data);
//   vfs_msg->flags=2;
//   return;
// }

// char* initrd_gets(vfs_message* vfs_msg) {
//   char str[256];
//   serial_print("File pos initrd:");
//   int_to_ascii(vfs_msg->pos,str);
//   serial_print(str);
//   serial_print("\n");
//   long max_data=size-vfs_msg->pos;
//   serial_print("max_data=");
//   int_to_ascii(max_data,str);
//   serial_print(str);
//   serial_print("\n");
//   serial_print("Amount requested:");
//   int_to_ascii(vfs_msg->data,str);
//   serial_print(str);
//   serial_print("\n");
//   if (vfs_msg->data>max_data) {
//     serial_print("OVER\n");
//     vfs_msg->flags=2;
//     return NULL;
//   }
//   char* data=malloc(sizeof(char)*vfs_msg->data);
//   for (long i=0;i<vfs_msg->data;i++) {
//     data[i]=(char)initrd[i+vfs_msg->pos];
//     // if (i<sizeof(vfs_message)) {
//     //   serial_print("data[");
//     //   char str[256];
//     //   int_to_ascii(i,str);
//     //   serial_print(str);
//     //   serial_print("]=");
//     //   hex_to_ascii((char)data[i],str);
//     //   serial_print(str);
//     //   serial_print("\n");
//     // }
//   }
//   vfs_msg->flags=0;
//   return data;
// }

// void process_vfs_msg(vfs_message* vfs_msg, uint32_t from) {
//   char* gets_data;
//   switch (vfs_msg->type) {
//     case VFS_PUTS:
//     initrd_puts(vfs_msg);
//     break;
//     case VFS_GETS:
//     gets_data=initrd_gets(vfs_msg);
//     break;
//     break;
//     default:
//     vfs_msg->flags=1;
//   }
//   Message msg;
//   msg.to=from;
//   msg.from=box;
//   msg.size=sizeof(vfs_message);
//   msg.msg=vfs_msg;
//   serial_print("Sending message with flags of ");
//   char str[256];
//   int_to_ascii(vfs_msg->flags,str);
//   serial_print(str);
//   serial_print("\n");
//   mailbox_send_msg(&msg);
//   if (gets_data) {
//     serial_print("Amount sending:");
//     char str[256];
//     int_to_ascii(vfs_msg->data,str);
//     serial_print(str);
//     serial_print("\n");
//     msg.size=vfs_msg->data;
//     msg.msg=gets_data;
//     mailbox_send_msg(&msg);
//     free(gets_data);
//   }
// }


// int main() {
//   size=initrd_sz();
//   initrd=malloc(size);
//   initrd_get(initrd);
//   box=mailbox_new(16,"initrd");
//   devfs_message* msg_data=malloc(sizeof(devfs_message));
//   msg_data->msg_type=DEVFS_REGISTER;
//   msg_data->dev_type=DEV_GLOBAL;
//   msg_data->mbox=box;
//   strcpy(&msg_data->name[0],"initrd");
//   Message msg;
//   msg.from=box;
//   msg.to=mailbox_find_by_name("devfs_register");
//   msg.size=sizeof(devfs_message);
//   msg.msg=msg_data;
//   mailbox_send_msg(&msg);
//   free(msg_data);
//   yieldToPID(DEVFS_PID);
//   for (;;) {
//     Message msg;
//     msg.msg=malloc(sizeof(vfs_message));
//     mailbox_get_msg(box,&msg,sizeof(vfs_message));
//     if (msg.from==0) {
//       free(msg.msg);
//     } else {
//       vfs_message* vfs_msg=(vfs_message*)msg.msg;
//       process_vfs_msg(vfs_msg,msg.from);
//       free(vfs_msg);
//       yieldToPID(VFS_PID);
//     }
//     yield();
//   }
// }
