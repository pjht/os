#ifndef TASKING_HELPERS_H
#define TASKING_HELPERS_H

#include "tasking.h"

void switchTask(Registers *from, Registers *to);
uint32_t readEip();

#endif
