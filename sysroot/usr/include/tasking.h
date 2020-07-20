#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>
#include <sys/types.h>

#ifndef INT_TASKING_H
typedef enum ThreadState {
  THREAD_RUNNING,
  THREAD_READY,
  THREAD_EXITED,
  THREAD_BLOCKED
} ThreadState;

#endif

void yield();
void yieldToPID(uint32_t pid);
void createProcCr3(void* start,void* cr3);
void createProcCr3Param(void* start,void* cr3,uint32_t param1,uint32_t param2);
char isPrivleged(uint32_t pid);
void blockThread(ThreadState state);
void unblockThread(pid_t pid,uint32_t tid);


#endif
