#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>
#include <sys/types.h>

#ifndef INT_TASKING_H
typedef enum TaskState {
  TASK_RUNNING,
  TASK_READY,
  TASK_EXITED,
  TASK_BLOCKED
} TaskState;

#endif

void yield();
void yieldToPID(uint32_t pid);
void createTask(void* task);
void createTaskCr3(void* task,void* cr3);
void createTaskCr3Param(void* task,void* cr3,uint32_t param1,uint32_t param2);
char isPrivleged(uint32_t pid);
void blockTask(TaskState state);
void unblockTask(pid_t pid);


#endif
