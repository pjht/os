#ifndef TASKING_H
#define TASKING_H
#include "i386/tasking.h"

void tasking_init();
void yield();
Task* createTask(void* eip);
void send_msg(uint32_t pid,char* msg);
char* get_msg(uint32_t* sender);
#endif
