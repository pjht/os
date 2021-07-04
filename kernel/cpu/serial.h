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
 * Write a character to serial port 0
 * \param c The character to write
*/
void serial_putc(char c);

/**
 * Read a character from serial port 0
 * \return The character read from the serial port
*/
char serial_getc();

/**
 * Write a character to the specified serial port
 * \param c The character to write
 * \param port The port number to write to
*/
void serial_putc_port(char c, int port);

/**
 * Read a character from the specified serial port
 * \return The character read from the serial port
 * \param port The port number to read from
*/
char serial_getc_port(int port);

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
