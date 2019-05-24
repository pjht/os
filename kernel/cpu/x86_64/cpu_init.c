#include "paging.h"
#include "isr.h"
#include <grub/multiboot2.h>
#include "pmem.h"
// #include "../tasking.h"

void cpu_init(struct multiboot_boot_header_tag* mbd) {
  isr_install();
  asm volatile("sti");
  pmem_init(mbd);
  paging_init();
  // tasking_init();
}
