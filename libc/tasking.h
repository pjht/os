/**
 * \file 
*/

#ifndef TASKING_H
#define TASKING_H

#include <stdint.h>
#include <sys/types.h>

#ifndef KERN_TASKING_H
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

/**
 * Yield the CPU to another process
*/
void yield();

/**
 * Create a process
 * \param start The start function of the process
 * \param cr3 The address space of the process
*/
void createProcCr3(void* start,void* cr3);
/**
 * Create a process with 2 arguments
 * \param start The start function of the process
 * \param cr3 The address space of the process
 * \param param1 The first parameter of the process
 * \param param2 The second parameter of the process
*/
void createProcCr3Param(void* start,void* cr3,void* param1,void* param2);
/**
 * Block the current thread
 * \param state The state to block it with
*/
void blockThread(thread_state state);
/**
 * Unblock a thread in a process
 * \param pid The PID of the thread's process
 * \param tid The TID of the thread
*/
void unblockThread(pid_t pid,pid_t tid);


#endif
