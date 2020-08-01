/**
 * \file 
*/

#ifndef PMEM_H
#define PMEM_H

#include <grub/multiboot2.h>

/**
 * Initialize the physical memory manager
 * \param tags The multiboot header
*/
void pmem_init(struct multiboot_boot_header_tag* tags);
/**
 * Allocate physical frames
 * \param num_pages The number of frames to allocate
 * \return the physical address of the allocated frames
*/
void* pmem_alloc(int num_pages);

/**
 * Free allocated physical frames
 * \param start The address to start freeing at.
 * \param num_pages The number of frames to free
*/
void pmem_free(void* start,int num_pages);

#endif
