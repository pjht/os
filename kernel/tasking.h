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
  int num_threads; //!< The number of threads in this process
  int num_threads_blocked; //!< The number of blocked threads in this process
  struct Thread* first_thread; //!< A pointer to the head of the linked list of threads for this process.
} Process;

/**
 * Represents a thread of a process
*/
typedef struct Thread {
  void* kernel_esp; //!< The thread's kernel stack.
  void* kernel_esp_top; //!< The top of the thread's kernel stack.
  void* address_space; //!< The address space of this thread. (it is in here and not in the process to simplify the task switch asembly)
  pid_t tid; //!< The TID of this thread.
  thread_state state; //!< The state of this thread. (running,ready to run,blocked,etc.)
  int errno; //!< The errno value for this thread.
  struct Thread* next_thread_in_process; //!< The next thread in the process.
  struct Thread* prev_thread_in_process; //!< The previous thread in the process.
  struct Thread* next_ready_to_run; //!< If the thread is in the ready to run list, this is the next ready to run thread. (potentially in a different process)
  struct Thread* prev_ready_to_run; //!< If the thread is in the ready to run list, this is the previous ready to run thread. (potentially in a different process)
  Process* process; //!< The thread's process.
} Thread;

extern Thread* current_thread;

/**
 * Create a task
 * \param eip The start address of the task
 * \param address_space The address space of the task
 * \param kmode Whether the task is a kernel mode task
 * \param param1 The thread's start function first parameter
 * \param param2 The thread's start function second parameter
 * \param isThread Whether we are creating a new process or a thread in a process. If we are creating a theead, param2_arg becomes the PID for the newly created thread.
*/
void tasking_create_task(void* eip,void* address_space,char kmode,void* param1,void* param2,char isThread);
/**
 * Initialize tasking
*/
void tasking_init();
/** 
 * Check whether the current process is privleged
 * \return whether the current process is privleged
*/
char tasking_is_privleged();
/** 
 * Get the PID of the current thread.
 * \return The current thread's PID
*/
pid_t tasking_get_PID();
/** 
 * Get the adddress of errno for the current thread
 * \return The address of errno
*/
int* tasking_get_errno_address();
/** 
 * Create a new thread
 * \param start The start address of the task
 * \param pid The PID that gets the new thread
 * \param param The thread's start function parameter
 * \return the TID of the thread
*/
pid_t tasking_new_thread(void* start,pid_t pid,void* param);

/**
 * Terminate the current thread
 * If the main thread terminates, the whole process terminates.
 * \note Currently, calling tasking_exit from any thread terminates the whole process.
 * \param code The exit code of the thread
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
