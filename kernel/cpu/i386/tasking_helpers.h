#ifndef TASKING_HELPERS_H
#define TASKING_HELPERS_H

#include "tasking.h"

void switchTask(uint32_t stack);
uint32_t readEip();

#endif
