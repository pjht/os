#ifndef TASKING_H
#define TASKING_H
#include <stdint.h>

typedef struct {
  int eax,ebx,ecx,edx;
  int esi,edi,ebp,esp;
  int eip,flags;
  uint32_t* page_dir;
} Task;

#endif
