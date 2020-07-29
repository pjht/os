#include "cpu/paging.h"
#include "cpu/arch_consts.h"
#include <stddef.h>

void address_spaces_copy_data(void* address_space, void* data,size_t size,void* virt_addr) {
  void* phys_addr=virt_to_phys(data);
  RUN_IN_ADDRESS_SPACE(address_space,map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1));
}

void* address_spaces_put_data(void* address_space, void* data,size_t size) {
  void* phys_addr=virt_to_phys(data);
  void* virt_addr;
  RUN_IN_ADDRESS_SPACE(address_space,{
   virt_addr=find_free_pages((size/PAGE_SZ)+1);
   map_pages(virt_addr,phys_addr,(size/PAGE_SZ)+1,1,1);
  });
  return virt_addr;
}
