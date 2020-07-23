// #include "cpu/halt.h"
// #include "cpu/i386/address_spaces.h"
// #include "cpu/i386/serial.h"
// #include "cpu/tasking.h"
// #include "rpc.h"
// #include <string.h>
// void rpc_init_struct(ThreadRPCStruct* info) {
//   for (size_t i = 0; i < 32; i++) {
//     info->funcs[i].code=NULL;
//     memset(info->funcs[i].name,'\0',32);
//   }
//   info->rpc_response=NULL;
//   info->num_funcs=0;
// }

// void rpc_reg_func(void* (*code)(void*),char* name) {
//   if (strlen(name)>31) {
//     serial_printf("Max length for RPC function name is 31!\n");
//     halt();
//   }
//   ThreadRPCStruct* info=tasking_get_rpc_struct(0);
//   if (info->num_funcs==32) {
//     serial_printf("Maximum # of RPC functions registered\n");
//     halt();
//   }
//   info->funcs[info->num_funcs].code=code;
//   strcpy(info->funcs[info->num_funcs].name,name);
//   info->num_funcs++;
// }

// void rpc_call(char* name,pid_t pid,void* data,size_t size) {
//   if (strlen(name)>31) {
//     serial_printf("Max length for RPC function name is 31!\n");
//     halt();
//   }
//   ThreadRPCStruct* info=tasking_get_rpc_struct(pid);
//   int func_idx=-1;
//   for (size_t i=0;i<info->num_funcs;i++) {
//     if (strcmp(info->funcs[i].name,name)==0) {
//       func_idx=i;
//     }
//   }
//   if (func_idx==-1) {
//     serial_printf("No such rpc function %s for PID %d",name,pid);
//   }
//   void* copieddata=address_spaces_put_data(currentThread->cr3,data,size);
//   tasking_new_thread(info->funcs[func_idx].code,pid,1,copieddata);
// }
