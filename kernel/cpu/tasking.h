#ifndef CPU_TASKING_H
#define CPU_TASKING_H

#include "i386/tasking.h"
#include "i386/isr.h"
#include <sys/types.h>

void tasking_init();
void tasking_yield(pid_t pid); //set pid to 0 for normal scheduling
Task* tasking_createTask(void* eip);
Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg);
char isPrivleged(uint32_t pid);
uint32_t getPID();
void tasking_exit(uint8_t code);
void tasking_block(TaskState newstate);
void tasking_unblock(pid_t pid);
int* tasking_get_errno_address();

#endif
