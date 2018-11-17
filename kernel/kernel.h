#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define MMAP_ENTRIES 5
extern uint32_t total_mb;
extern uint32_t mem_map[MMAP_ENTRIES+1][2];

void user_input(char* str);
void kgets(char* buf);
#endif
