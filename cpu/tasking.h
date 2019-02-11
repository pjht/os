#ifndef CPU_TASKING_H
#define CPU_TASKING_H
#include "i386/tasking.h"

void tasking_init();
void tasking_yield();
Task* tasking_createTask(void* eip);
void tasking_send_msg(uint32_t pid,char* msg);
char* tasking_get_msg(uint32_t* sender);
#endif
