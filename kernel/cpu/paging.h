/**
 * \file 
*/

#ifndef PAGING_H
#define PAGING_H

/**
 * Map virtual pages to physical frames.
 * \param virt_addr_ptr The start of the virtual range to map.
 * \param phys_addr_ptr The start of the physical range to map.
 * \param num_pages The number of pages to map.
 * \param usr Are the pages acessible by user mode code
 * \param wr Are the pages writable by user mode code (kernel always has write permissions)
*/
void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr);
/**
 * Unmap virtual pages,
 * \param start_virt The start of the virtual range to unmap.
 * \param num_pages The number of pages to map.
*/
void unmap_pages(void* start_virt,int num_pages);
/**
 * Allocate virtual pages & map them to physical memory.
 * \param num_pages The number of pages to allocate.
 * \return a pointer to the allocated pages.
*/
void* alloc_pages(int num_pages);
/**
 * Allocate virtual pages at a specific address & map them to physical memory.
 * \param num_pages The number of pages to allocate.
 * \param addr The adress to start allocation at.
*/
void alloc_pages_virt(int num_pages,void* addr);
/**
 * Initialize paging
*/
void paging_init();
/**
 * Create a new address space
 * \return a pointer to the new address space in physical memory.
*/
void* paging_new_address_space();
/**
 * Load an address space
 * \param address_space The address space to load
*/
void load_address_space(void* address_space);
/**
 * Convert a virtual address to a physical one.
 * \param virt_addr The virtual address to convert
 * \return the physical adress it maps to, or NULL if it is not mapped.
*/
void* virt_to_phys(void* virt_addr);
/**
 * Finds free virtual pages and returns the start address
 * \param num_pages The minimum size of the free area
 * \return the start of the free area
*/
void* find_free_pages(int num_pages);

/**
 * Get the current address space
 * \return a pointer to the current address space in physical memory.
*/
void* get_address_space();

#endif
