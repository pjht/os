#ifndef RPC_H
#define RPC_H

typedef struct RPCFuncInfo {
  char name[32];
  void* (*code)(void*);
} RPCFuncInfo;

typedef struct TaskRPCStruct {
  RPCFuncInfo funcs[32];
  int next_func;
  void* rpc_response;
} ThreadRPCStruct;


void rpc_init_struct(ThreadRPCStruct* info);

#endif
