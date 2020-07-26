/**
 * \file 
*/

#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0 //!< Success exit code
#define EXIT_FAILURE 1 //!< Failure exit code

/**
 * Allocates a block of memory on the heap
 * \param size The size of the block to allocate
 * \return The address of the allocated block, or NUL if the allocation failed
*/
void* malloc(size_t size);
/**
 * Changes a block of memory on the heap to a new size, or if mem is NULL, act like malloc
 * \param mem The block of memory to change size
 * \param new_sz The size of the block to allocate
 * \return The new address of the allocated block, or NUL if the allocation failed
*/
void* realloc(void *mem, size_t new_sz);

/**
 * Frees a block of memory on the heap
 * \param mem The block of memory to free
*/
void free(void* mem);
#ifndef DOXYGEN_SHOULD_SKIP_THIS
void abort(void);               // GCC required
int atexit(void (*func)(void)); // GCC required
int atoi(const char *str);      // GCC required
char *getenv(const char *name); // GCC required
#endif
/**
 * Exit the process
 * \param code The exit code of the process
*/
__attribute__((noreturn)) void exit(int code);

#endif
