/**
 * \file 
*/

#ifndef SERIAL_H
#define SERIAL_H

/**
 * Initialize the serial driver 
*/
void serial_init();

/**
 * Write a character to the serial port
 * \param c The character to write
*/
void serial_putc(char c);

/**
 * Write a string to the serial port
 * \param s The string to write
 * \note This function is provided by platform-independent code, a serial driver does not need to implement this.
*/
void serial_write_string(const char* s); 

/**
 * Printf, but to the serial port
 * \param format The format string
 * \param ... Arguments for the format string
 * \note This function is provided by platform-independent code, a serial driver does not need to implement this.
*/
void serial_printf(const char* format,...);

#endif
