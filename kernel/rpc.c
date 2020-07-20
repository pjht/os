// #include "cpu/tasking.h"
// #include <string.h>
// #include "rpc.h"
// #include "cpu/i386/serial.h"
// #include "cpu/halt.h"
// void rpc_init_struct(TaskRPCStruct* info) {
//   for (size_t i = 0; i < 32; i++) {
//     info->funcs[i].code=NULL;
//     memset(info->funcs[i].name,'\0',32);
//   }
//   info->rpc_response=NULL;
//   info->next_func=0;
// }

// void rpc_reg_func(void* (*code)(void*),char* name) {
//   if (strlen(name)>31) {
//     serial_printf("Max length for RPC function name is 31!\n");
//     halt();
//   }
//   TaskRPCStruct* info=tasking_get_rpc_struct(0);
//   if (info->next_func==32) {
//     serial_printf("Maximum # of RPC functions registered\n");
//     halt();
//   }
//   info->funcs[info->next_func].code=code;
//   strcpy(info->funcs[info->next_func].name,name);
//   info->next_func++;
// }

// void rpc_call(char* name,pid_t pid,void* data) {
//   if (strlen(name)>31) {
//     serial_printf("Max length for RPC function name is 31!\n");
//     halt();
//   }
//   TaskRPCStruct* info=tasking_get_rpc_struct(pid);
//   int func_idx;
// }
