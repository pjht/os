#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_COM1_BASE 0x3F8      /* COM1 base port */

void serial_full_configure(unsigned short com, unsigned short divisor);
void serial_write_string(unsigned short com, char *str);
#endif
