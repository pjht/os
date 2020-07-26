/**
 * \file 
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define BLK_SZ 4096 //!< Page size of the architecture

/**
 * Allocates pages of memory
 * \param num_pages The number of pages to allocate
 * \return the start address of the pages
*/ 
void* alloc_memory(int num_pages);

/**
 * Allocates pages of memory at a specified start address
 * \param num_pages The number of pages to allocate
 * \param addr The start address of the pages
*/ 
void alloc_memory_virt(int num_pages,void* addr);

/**
 * Creates a new address space with kernel mappings
 * \return a pointer to the new address space in physical memory
*/ 
void* new_address_space();

/**
 * Copy data into an address space at a specified virtual address
 * \param cr3 The adress space to copy data to.
 * \param data The data to copy
 * \param size The size of the data
 * \param virt_addr The address to copy the data to in the address space 
*/
void copy_data(void* cr3, void* data,size_t size,void* virt_addr);

/**
 * Put data into an address space at an unknown virtual address
 * \param cr3 The adress space to copy data to.
 * \param data The data to copy
 * \param size The size of the data
 * \return The address that the data was copied to.
*/
void* put_data(void* cr3, void* data,size_t size);

/**
 * Map physical pages into virtual memory
 * \param phys_addr the start of the physical memory block to map
 * \param num_pages the number of pages to map
 * \return the start address of the mapping in virtual memory
*/
void* map_phys(void* phys_addr,size_t num_pages);

#endif
