// #include "gdt.h"
// #include "paging.h"
#include "isr.h"
#include <grub/multiboot.h>
// #include "pmem.h"
// #include "../tasking.h"

void cpu_init(multiboot_info_t* mbd) {
  // gdt_init();
  isr_install();
  asm volatile("sti");
  // pmem_init(mbd);
  // paging_init();
  // tasking_init();
}
