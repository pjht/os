#ifndef PMEM_H
#define PMEM_H

#include <grub/multiboot.h>

void pmem_init(multiboot_info_t* mbd);
void* pmem_alloc(int num_pages);
void pmem_free(int start_page,int num_pages);

#endif
