#ifndef CPU_TASKING_H
#define CPU_TASKING_H

#include "i386/tasking.h"
#include "i386/isr.h"

void tasking_init();
void tasking_yield();
Task* tasking_createTask(void* eip);
Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg);
char isPrivleged(uint32_t pid);
uint32_t getPID();
#endif
