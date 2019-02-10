#include "gdt.h"
#include "paging.h"
#include "isr.h"

void cpu_init() {
  gdt_init();
  isr_install();
  asm volatile("sti");
  paging_init();
}
