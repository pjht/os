/**
 * \file 
*/

#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>


/**
 * Allocate a block in the kernel heap
 * \param size The size of the block
 * \return the address of the block in the heap.
*/
void* kmalloc(size_t size);

/**
 * Free a block in the kernel heap
 * \param mem The address of the block
*/
void kfree(void* mem);

#endif
