/**
 * \file 
*/

#ifndef VGA_ERR_H
#define VGA_ERR_H

/**
 * Initilaze the VGA error writing driver
*/
void vga_init(char* screen);

/**
 * Write a string starting at the top line of the VGA display
*/
void vga_write_string(const char *string);

#endif
