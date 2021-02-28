/**
 * \file 
*/

#ifndef KERN_TASKING_H
#define KERN_TASKING_H

#include <stdint.h>
#include <sys/types.h>
#include "rpc.h"

#ifndef TASKING_H

/**
 * Represents the state of a thread
*/
typedef enum thread_state {
  THREAD_RUNNING, //!< The state of a running thread
  THREAD_READY,  //!< The state of a ready to run thread
  THREAD_EXITED,  //!< The state of an exited thread
  THREAD_BLOCKED,  //!< The state of a generically blocked thread
  THREAD_WAITING_FOR_RPC, //!< The state of a thread waiting for an RPC call to return
  THREAD_WAITING_FOR_RPC_INIT //!< The state of a thread waiting for a process to fully initilaize it's RPC functions
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
  pid_t rpc_calling_pid; //!< The PID of the thread that called this RPC (only used for RPC handler threads)
  pid_t rpc_calling_tid; //!< The TID of the thread that called this RPC (only used for RPC handler threads)
  void* rpc_ret_buf; //!< The return buffer of the RPC call that the thread made
} __attribute__((packed)) Thread;

extern Thread* current_thread;

/**
 * Create a task
 * \param eip The start address of the task
 * \param address_space The address space of the task
 * \param kmode Whether the task is a kernel mode task
 * \param param1 The thread's start function first parameter
 * \param param2 The thread's start function second parameter
 * \param isThread Whether we are creating a new process or a thread in a process. If we are creating a theead, param2_arg becomes the PID for the newly created thread.
 * \param is_irq_handler Whether the new task is an irq handler task
*/
void tasking_create_task(void* eip,void* address_space,char kmode,void* param1,void* param2,char isThread,char is_irq_handler);
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
 * Get the TID of the current thread.
 * \return The current thread's TID
*/
pid_t tasking_get_TID();
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
 * \param is_irq_handler Whether the new thread is an irq handler thread
 * \return the TID of the thread
*/
pid_t tasking_new_thread(void* start,pid_t pid,void* param,char is_irq_handler);

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
/**
 * Get the address_space of a process
 * \param pid the PID of the process
 * \return the address_space of the process
*/
void* tasking_get_address_space(pid_t pid);

/**
 * Set the RPC calling thread for an RPC handler thread to the current threasd
 * \param pid The PID of the handler thread
 * \param tid The TID of the handler thread
*/
void tasking_set_rpc_calling_thread(pid_t pid,pid_t tid);

/**
 * Get the RPC calling thread for the current thread
 * \param tid A pointer to a pid_t to store the return TID
 * \return the return PID
 * \note This is only applicable for an RPC handler thread
*/
pid_t tasking_get_rpc_calling_thread(pid_t* tid);
/** 
 * Set the RPC return buffer for the calling thread
 * \param buf The return buffer
*/
void tasking_set_rpc_ret_buf(void* buf);

/** 
 * Get the RPC return buffer for the current thread
 * \return the return buffer
*/
void* tasking_get_rpc_ret_buf();

/** 
 * Terminate the current thread
*/
void tasking_thread_exit();

/**
 * Check if a process exists
 * \param pid The param of the process to check
 * \return Whether the process exists
*/
char tasking_check_proc_exists(pid_t pid);

#endif
