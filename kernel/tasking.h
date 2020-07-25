/**
 * \file 
*/

#ifndef KERN_TASKING_H
#define KERN_TASKING_H

#include <stdint.h>
#include <sys/types.h>

#ifndef TASKING_H

/**
 * Represents the state of a thread
*/
typedef enum thread_state {
  THREAD_RUNNING, //!< The state of a running thread
  THREAD_READY,  //!< The state of a ready to run thread
  THREAD_EXITED,  //!< The state of an exited thread
  THREAD_BLOCKED  //!< The state of a generically blocked thread
} thread_state;

#endif

struct Thread;

/**
 * Represents a process
*/
typedef struct Process {
  char priv; //!< Whether the process is privileged (can execute syscalls to acesss all of memory/has acess to IO ports).
  pid_t pid; //!< The PID of this process
  pid_t next_tid; //!< The TID that the next created thread will use.
  int numThreads; //!< The number of threads in this process
  int numThreadsBlocked; //!< The number of blocked threads in this process
  struct Thread* firstThread; //!< A pointer to the head of the linked list of threads for this process.
} Process;

/**
 * Represents a thread of a process
*/
typedef struct Thread {
  void* kernel_esp; //!< The thread's kernel stack.
  void* kernel_esp_top; //!< The top of the thread's kernel stack.
  void* cr3; //!< The address space of this thread. (it is in here and not in the process to simplify the task switch asembly)
  pid_t tid; //!< The TID of this thread.
  thread_state state; //!< The state of this thread. (running,ready to run,blocked,etc.)
  int errno; //!< The errno value for this thread.
  struct Thread* nextThreadInProcess; //!< The next thread in the process.
  struct Thread* prevThreadInProcess; //!< The previous thread in the process.
  struct Thread* nextReadyToRun; //!< If the thread is in the ready to run list, this is the next ready to run thread. (potentially in a different process)
  struct Thread* prevReadyToRun; //!< If the thread is in the ready to run list, this is the previous ready to run thread. (potentially in a different process)
  Process* process; //!< The thread's process.
} Thread;

extern Thread* current_thread;

/**
 * Create a task
 * \param eip The start address of the task
 * \param cr3 The address space of the task
 * \param kmode Whether the task is a kernel mode task
 * \param param1_exists Whether param1_arg is a valid value
 * \param param1_arg The thread's start function first parameter
 * \param param2_exists Whether param2_arg is a valid value
 * \param param2_arg The thread's start function second parameter/
 * \param isThread Whether we are creating a new process or a thread in a process. If we are creating a theead, param2_arg becomes the PID for the newly created thread, and param2_exists must be 0.
*/
void tasking_create_task(void* eip,void* cr3,char kmode,char param1_exists,void* param1_arg,char param2_exists,void* param2_arg,char isThread);
/**
 * Initialize tasking
*/
void tasking_init();
/** 
 * Check whether the current process is privleged
*/
char tasking_is_privleged();
/** 
 * Get the PID of the current thread.
*/
pid_t tasking_get_PID();
/** 
 * Get the adddress of errno for the current thread
*/
int* tasking_get_errno_address();
/** 
 * Create a new thread
 * \param start The start address of the task
 * \param pid The PID that gets the new thread
 * \param param_exists Whether param_arg is a valid value
 * \param param_arg The thread's start function parameter
 * \return the TID of the thread
*/
pid_t tasking_new_thread(void* start,pid_t pid,char param_exists,void* param_arg);

/**
 * Terminate the current thread
 * If the main thread terminates, the whole process terminates.
 * \note Currently, calling tasking_exit from any thread terminates the whole process.
*/
void tasking_exit(int code);
/**
 * Block the current thread & yield
 * \param newstate The state to block it in
*/
void tasking_block(thread_state newstate);
/**
 * Unblock a thread
 * \param pid The PID that contains the thread to unblock
 * \param tid The TID in the process to unblock.
*/
void tasking_unblock(pid_t pid,pid_t tid);
/**
 * Yield to the next ready thread in any process
*/
void tasking_yield();

#endif
