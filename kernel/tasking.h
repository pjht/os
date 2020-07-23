#ifndef KERN_TASKING_H
#define KERN_TASKING_H

#include <stdint.h>
#include <sys/types.h>

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

extern Thread* currentThread;

void tasking_createTask(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg,char isThread);
void tasking_init();
char tasking_isPrivleged();
pid_t tasking_getPID();
int* tasking_get_errno_address();
uint32_t tasking_new_thread(void* start,pid_t pid,char param_exists,uint32_t param_arg);

void tasking_exit(uint8_t code);
void tasking_block(ThreadState newstate);
void tasking_unblock(pid_t pid,uint32_t tid);
void tasking_yield();

#endif
