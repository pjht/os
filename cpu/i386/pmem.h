#ifndef PMEM_H
#define PMEM_H

#include <grub/multiboot2.h>

void pmem_init(struct multiboot_boot_header_tag* tags);
void* pmem_alloc(int num_pages);
void pmem_free(int start_page,int num_pages);

#endif
