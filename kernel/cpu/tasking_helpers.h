/**
 * \file 
*/

#ifndef TASKING_HELPERS_H
#define TASKING_HELPERS_H

#include "../tasking.h"

/**
 * The assembly part of switching to a thread. Performs the actual context switch.
 * \param thread The thread to switch to.
*/
void switch_to_thread_asm(Thread* thread);

/**
 * Initializes a usermode task
*/
void task_init();

/**
 * An assembly helper for waiting for an unblocked thread
 * Starts interrupts, halts, then clears interrupts.
*/
void wait_for_unblocked_thread_asm();

/**
 * Setup a kernel stack for a thread
 * \param thread The thread to setup a stack for
 * \param param1 The thread's start function first parameter
 * \param param2 The thread's start function second parameter
 * \param kmode Whether the thread is a kernel mode thread
 * \param eip The start address of the thread
*/
void setup_kstack(Thread* thread,void* param1,void* param2,char kmode,void* eip);

/**
 * Frees a kernel stack so it can be used again
 * \param stack_ptr The kernel stack to free
*/
void free_kstack(void* stack_ptr);

#endif
