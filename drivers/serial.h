#ifndef SERIAL_H
#define SERIAL_H
#include <stdint.h>

void serial_configure(uint32_t com, uint32_t rate);
void serial_write_string(uint32_t com, char *str);
#endif
