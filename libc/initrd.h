/**
 * \file 
*/

#ifndef INITRD_H
#define INITRD_H


/**
 * Gets the size of the initrd
 * \return the size of the initrd
*/ 
long initrd_sz(); 
/**
 * Copies the initrd to a specified area in memory
 * \param initrd the address to copy the initrd to
*/ 
void initrd_get(char* initrd);

#endif
