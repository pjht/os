#ifndef SERIAL_H
#define SERIAL_H

void serial_init();
void serial_write_string(const char* s);
void serial_printf(const char* format,...);

#endif
