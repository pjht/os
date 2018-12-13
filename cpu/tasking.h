#ifndef TASKING_H
#define TASKING_H
#include <stdint.h>

void tasking_init();
uint32_t fork();
void yield();

#endif
