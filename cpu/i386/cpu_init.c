#include "gdt.h"
#include "paging.h"
void cpu_init() {
  gdt_init();
  paging_init();
}
