/**
 * \file 
*/

#ifndef ADDRESS_SPACES_H
#define ADDRESS_SPACES_H

/**
 * Copy data into an address space at a specified virtual address
 * \param address_space The adress space to copy data to.
 * \param data The data to copy
 * \param size The size of the data
 * \param virt_addr The address to copy the data to in the address space 
*/
void address_spaces_copy_data(void* address_space, void* data,uint32_t size,void* virt_addr);

/**
 * Put data into an address space at an unknown virtual address
 * \param address_space The adress space to copy data to.
 * \param data The data to copy
 * \param size The size of the data
 * \return The address that the data was copied to.
*/
void* address_spaces_put_data(void* address_space, void* data,uint32_t size);
#endif
