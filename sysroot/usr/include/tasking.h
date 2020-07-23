#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>
#include <sys/types.h>

#ifndef KERN_TASKING_H
typedef enum ThreadState {
  THREAD_RUNNING,
  THREAD_READY,
  THREAD_EXITED,
  THREAD_BLOCKED
} ThreadState;

#endif

void yield();
void yieldToPID(pid_t pid);
void createProcCr3(void* start,void* cr3);
void createProcCr3Param(void* start,void* cr3,void* param1,void* param2);
char isPrivleged(pid_t pid);
void blockThread(ThreadState state);
void unblockThread(pid_t pid,pid_t tid);


#endif
