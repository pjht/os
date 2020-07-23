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
  pid_t pid;
  pid_t next_tid;
  int numThreads;
  int numThreadsBlocked;
  struct Thread* firstThread;
} Process;

typedef struct Thread {
  void* kernel_esp;
  void* kernel_esp_top;
  void* cr3; //In thread to make the task switch asm easier
  pid_t tid;
  ThreadState state;
  int errno;
  struct Thread* nextThreadInProcess;
  struct Thread* prevThreadInProcess;
  struct Thread* nextReadyToRun;
  struct Thread* prevReadyToRun;
  Process* process;
} Thread;

extern Thread* currentThread;

void tasking_createTask(void* eip,void* cr3,char kmode,char param1_exists,void* param1_arg,char param2_exists,void* param2_arg,char isThread);
void tasking_init();
char tasking_isPrivleged();
pid_t tasking_getPID();
int* tasking_get_errno_address();
pid_t tasking_new_thread(void* start,pid_t pid,char param_exists,void* param_arg);

void tasking_exit(int code);
void tasking_block(ThreadState newstate);
void tasking_unblock(pid_t pid,pid_t tid);
void tasking_yield();

#endif
