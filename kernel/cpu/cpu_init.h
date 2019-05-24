#ifndef CPU_INIT_H
#define CPU_INIT_H

#include <grub/multiboot2.h>

void cpu_init(struct multiboot_boot_header_tag* tags);

#endif
