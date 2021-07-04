/**
 * \file 
*/


#ifndef DBG_H
#define DBG_H

/**
 * Prints a string to the kernel serial console
 * \param str The string to print
*/ 
void serial_print(char* str);

/**
 * Write a character to the specified serial port
 * \param c The character to write
 * \param port The port number to write to
*/
void user_serial_putc(char c, int port);

/**
 * Read a character from the specified serial port
 * \return The character read from the serial port
 * \param port The port number to read from
*/
char user_serial_getc(int port);
#endif
