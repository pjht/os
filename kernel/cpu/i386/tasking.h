#ifndef INT_TASKING_H
#define INT_TASKING_H

#include <stdint.h>

#ifndef TASKING_H
typedef enum ThreadState {
  THREAD_RUNNING,
  THREAD_READY,
  THREAD_EXITED,
  THREAD_BLOCKED
} ThreadState;
#endif

struct Thread;

typedef struct Process {
  char priv;
  uint32_t pid;
  uint32_t next_tid;
  int numThreads;
  int numThreadsBlocked;
  struct Thread* firstThread;
} Process;

typedef struct Thread {
  uint32_t kernel_esp;
  uint32_t kernel_esp_top;
  void* cr3; //In thread to make the task switch asm easier
  uint32_t tid;
  ThreadState state;
  int errno;
  struct Thread* nextThreadInProcess;
  struct Thread* prevThreadInProcess;
  struct Thread* nextReadyToRun;
  struct Thread* prevReadyToRun;
  Process* process;
} Thread;

#endif
