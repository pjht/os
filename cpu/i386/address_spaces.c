#include "paging.h"

void copy_data(void* cr3, void* data,uint32_t size,void* virt_addr) {
  uint32_t old_cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old_cr3)::"%eax");
  void* phys_addr=virt_to_phys(data);
  load_address_space(cr3);
  map_pages((void*)((uint32_t)virt_addr<<12),phys_addr,(size/4096)+1,1,0);
  load_address_space(old_cr3);
}
