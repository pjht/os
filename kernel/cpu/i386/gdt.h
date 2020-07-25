/**
 * \file 
*/

#ifndef GDT_H
#define GDT_H

/**
 * Initializes the GDT & TSS.
*/

void gdt_init();

/**
 * Allows all ports in the IOPB. 
*/


void allow_all_ports();

/**
 * Blocks all ports in the IOPB. 
*/

void block_all_ports();
#endif
