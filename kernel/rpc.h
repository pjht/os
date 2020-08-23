/**
 * \file 
*/

#ifndef KERN_RPC_H
#define KERN_RPC_H

#include <stddef.h>

typedef void (*rpc_func)(void*); //!< Type of an RPC function

/**
 * Represents an RPC fumctiom with name 
*/
typedef struct rpc_func_info {
  char name[32]; //!< THe name of the function
  rpc_func code; //!< A pointer to the code that implements the funtcion
} rpc_func_info;


/**
 * Call an RPC function
 * \param pid The PID of the process with the RPC function
 * \param name The name of the function to call
 * \param buf The argument buffer to provide
 * \param size The size of the argument buffer
 * \return the return buffer of the RPC functiom
*/
void* kernel_rpc_call(pid_t pid,char* name,void* buf,size_t size);
/**
 * Register an RPC function
 * \param name The name of the function
 * \param code The code of the function
*/
void kernel_rpc_register_func(char* name,rpc_func code);
/**
 * Deallocate an RPC return buffer
 * \param buf The buffer to deallocate
 * \param size The size of the buffer to deallocate
*/
void kernel_rpc_deallocate_buf(void* buf,size_t size);

/**
 * Set the RPC return buffer for the calling thread & unblock the calling thread
 * \param buf The return buffer
 * \param size The size of the return buffer
 * \note This function must only be called from an RPC thread
*/
void kernel_rpc_return(void* buf,size_t size);

/**
 * Get the number of RPC functions a process has registers
 * \param pid The PID of the process
 * \return the number of RPC functions the process has registered
*/
size_t kernel_get_num_rpc_funcs(pid_t pid);

/**
 * Mark the current process as ready to accept RPC calls
*/
void kernel_rpc_mark_as_init();

/**
 * Check if a process is ready to accept RPC calls
 * \param pid The pid to check
 * \return whether the process is ready
*/
char kernel_rpc_is_init(pid_t pid);

#endif
