#ifndef CPU_RAM_H
#define CPU_MEMORY_H

#include <stdint.h>
#define BLK_SZ 4096

void* alloc_memory(uint32_t blocks);

#endif
