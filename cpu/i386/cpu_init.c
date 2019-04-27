#include "gdt.h"
#include "paging.h"
#include "isr.h"
#include "pmem.h"
#include "../tasking.h"

void cpu_init(struct multiboot_boot_header_tag* tags) {
  gdt_init();
  isr_install();
  asm volatile("sti");
  pmem_init(tags);
  paging_init();
  tasking_init();
}
