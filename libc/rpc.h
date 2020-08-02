#ifndef RPC_H
#define RPC_H

#include <sys/types.h>
#include <stddef.h>

typedef void* (*rpc_func)(void*); //!< Type of an RPC function

/**
 * Call an RPC function
 * \param pid The PID of the process with the RPC function
 * \param name The name of the function to call
 * \param buf The argument buffer to provide
 * \param size The size of the argument buffer
 * \return the return buffer of the RPC functiom
*/
void* rpc_call(pid_t pid,char* name,void* buf,size_t size);

/**
 * Register an RPC function
 * \param name The name of the function
 * \param code The code of the function
*/
void rpc_register_func(char* name,rpc_func code);
/**
 * Deallocate an RPC return buffer
 * \param buf The buffer to deallocate
 * \param size The size of the buffer to deallocate
*/
void rpc_deallocate_buf(void* buf,size_t size);

/**
 * Set the RPC return buffer for the calling thread & unblock the calling thread
 * \param buf The return buffer
 * \param size The size of the return buffer
 * \note This function must only be called from an RPC thread
*/
void rpc_return(void* buf,size_t size);


/**
 * Mark the current process as ready to accept RPC calls
*/
void rpc_mark_as_init();

#endif
