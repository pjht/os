#ifndef CPU_TASKING_H
#define CPU_TASKING_H

#include "i386/tasking.h"
#include "i386/isr.h"
#include <sys/types.h>
#include "../rpc.h"

extern Thread* currentThread;

void tasking_createTask(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg,char isThread);
void tasking_init();
char tasking_isPrivleged();
pid_t tasking_getPID();
int* tasking_get_errno_address();
void tasking_new_thread(void* start,pid_t pid,char param_exists,uint32_t param_arg);

void tasking_exit(uint8_t code);
void tasking_block(ThreadState newstate);
void tasking_unblock(pid_t pid,uint32_t tid);
void tasking_yield();

#endif
