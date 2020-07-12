#ifndef RPC_H
#define RPC_H

typedef struct RPCFuncInfo {
  char name[32];
  char* (*code)(char*);
} RPCFuncInfo;

typedef struct TaskRPCStruct {
  int pendingrpc;
  int callingpid;
} TaskRPCStruct;


void rpc_init_struct(TaskRPCStruct* info);

#endif
