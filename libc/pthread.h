/**
 * \file 
*/

#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>

typedef pid_t pthread_t; //!< Represents a thread
typedef int pthread_attr_t; //!< Created as dummy
typedef int pthread_spinlock_t; //!< Represents a spinlock

/**
 * Create a thread in the current process
 * \param thread A pointer to a pthread_t where the created thread will be placed
 * \param attr Either a pointer to a thread attribute structure, or NULL, in which case the default attributes will be used
 * \param start_routine The start function of the thread
 * \param arg An arhument to the start function
 * \return 0 if creation was sucsessful, 1 otherwise
*/
int pthread_create(pthread_t *restrict thread,
       const pthread_attr_t *restrict attr,
       void *(*start_routine)(void*), void *restrict arg);

/**
 * Terminate the current thread 
 * \param value_ptr A pointer that when other threads in the process join on the current thread, they will recieve. (not implemented)
*/
void pthread_exit(void *value_ptr);

/**
 * Initializes a spin lock
 * \param lock The spinlock to initialize
 * \param pshared Unused
 * \returns 0 on success, 1 on failure
*/
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

/**
 * Locks a spin lock
 * \param lock The spinlock to lock
 * \returns 0 on success, 1 on failure
*/
int pthread_spin_lock(pthread_spinlock_t *lock);

/**
 * Unlocks a spin lock
 * \param lock The spinlock to unlock
 * \returns 0 on success, 1 on failure
*/
int pthread_spin_unlock(pthread_spinlock_t *lock);

/**
 * Destroys a spin lock
 * \param lock The spinlock to destroy
 * \returns 0 on success, 1 on failure
*/
int pthread_spin_destroy(pthread_spinlock_t *lock);



#endif
