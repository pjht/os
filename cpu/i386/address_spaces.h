#ifndef ADDRESS_SPACES_H
#define ADDRESS_SPACES_H

#include <stdint.h>

void copy_data(void* cr3, void* data,uint32_t size,void* virt_addr);

#endif
