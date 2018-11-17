#include "gdt.h"
#include "isr.h"
#include "paging.h"

void cpu_init() {
  init_gdt();
  isr_install();
  asm volatile("sti");
  init_paging();
}
