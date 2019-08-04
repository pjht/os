#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>

void yield();
void yieldToPID(uint32_t pid);
void createTask(void* task);
void createTaskCr3(void* task,void* cr3);
void createTaskCr3Param(void* task,void* cr3,uint32_t param1,uint32_t param2);
char isPrivleged(uint32_t pid);
void send_msg(uint32_t pid,void* msg,uint32_t size);
void* get_msg(uint32_t* sender,uint32_t* size);

#endif
