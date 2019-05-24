#ifndef CPU_TASKING_H
#define CPU_TASKING_H

#include "i386/tasking.h"
#include "i386/isr.h"

void tasking_init();
void tasking_yield();
Task* tasking_createTask(void* eip);
Task* tasking_createTaskCr3(void* eip,void* cr3);
char isPrivleged(uint32_t pid);
void tasking_send_msg(uint32_t pid,void* msg,uint32_t size);
void* tasking_get_msg(uint32_t* sender);
#endif
