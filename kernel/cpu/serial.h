#ifndef SERIAL_H
#define SERIAL_H

void serial_init();
void serial_putc(char c);
void serial_write_string(const char* s); //Provided by platform-independent code
void serial_printf(const char* format,...); //Provided by platform-independent code

#endif
