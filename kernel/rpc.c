#include "cpu/tasking.h"
#include <string.h>
#include "rpc.h"
void rpc_init_struct(TaskRPCStruct* info) {
  info->pendingrpc = 0;
  info->callingpid = 0;
}

