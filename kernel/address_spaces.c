#include "cpu/paging.h"
#include "cpu/arch_consts.h"

void address_spaces_copy_data(void* cr3, void* data,uint32_t size,void* virt_addr) {
  void* old_cr3=get_cr3();
  void* phys_addr=virt_to_phys(data);
  load_address_space(cr3);
  map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1);
  load_address_space(old_cr3);
}

void* address_spaces_put_data(void* cr3, void* data,uint32_t size) {
  void* old_cr3=get_cr3();
  void* phys_addr=virt_to_phys(data);
  load_address_space(cr3);
  void* virt_addr=find_free_pages((size/PAGE_SZ)+1);
  map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1);
  load_address_space(old_cr3);
  return virt_addr;
}
