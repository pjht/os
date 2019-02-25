#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>

void yield();
void createTask(void* task);
char isPrivleged(uint32_t pid);
void send_msg(uint32_t pid,char* msg);
char* get_msg(uint32_t* sender);

#endif
