#ifndef CPU_TASKING_H
#define CPU_TASKING_H

#include "i386/tasking.h"

void tasking_init();
void tasking_yield();
Task* tasking_createTask(void* eip);
char isPrivleged(uint32_t pid);
void send_msg(uint32_t pid,void* msg);
void* get_msg(uint32_t* sender);
#endif
