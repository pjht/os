#include "cpu/paging.h"
#include "cpu/arch_consts.h"
#include <stddef.h>

void address_spaces_copy_data(void* address_space, void* data,size_t size,void* virt_addr) {
  void* old_address_space=get_address_space();
  void* phys_addr=virt_to_phys(data);
  load_address_space(address_space);
  map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1);
  load_address_space(old_address_space);
}

void* address_spaces_put_data(void* address_space, void* data,size_t size) {
  void* old_address_space=get_address_space();
  void* phys_addr=virt_to_phys(data);
  load_address_space(address_space);
  void* virt_addr=find_free_pages((size/PAGE_SZ)+1);
  map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1);
  load_address_space(old_address_space);
  return virt_addr;
}
