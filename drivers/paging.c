#include <stdint.h>
#include "paging_helpers.h"
#include "paging.h"
uint32_t page_directory [1024] __attribute__((aligned(4096)));
uint32_t page_tables [1048576] __attribute__((aligned(4096)));
void* next_kern_virt=(void*)KERN_VIRT_START;
void* next_kern_phys=(void*)KERN_PHYS_START;
void alloc_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr,uint32_t* page_directory,uint32_t* page_tables) {
  uint32_t virt_addr=(uint32_t)virt_addr_ptr;
  uint32_t phys_addr=(uint32_t)phys_addr_ptr;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  int flags=1;
  flags=flags|((wr&1)<<1);
  flags=flags|((usr&1)<<2);
  for (int i=0;i<num_pages;i++) {
    page_tables[(dir_entry*1024)+table_entry]=phys_addr|flags;
    table_entry++;
    if (table_entry==1024) {
      table_entry=0;
      page_directory[dir_entry]=((uint32_t)&(page_tables[table_entry]))|flags;
      dir_entry++;
    } else if (i==num_pages) {
      page_directory[dir_entry]=((uint32_t)&(page_tables[table_entry]))|flags;
    }
  }
}

void* alloc_kern_pages(int num_pages,char wr) {
  void* starting=next_kern_virt;
  alloc_pages(next_kern_virt,next_kern_phys,num_pages,0,wr,page_directory,page_tables);
  next_kern_virt+=num_pages*4096;
  next_kern_phys+=num_pages*4096;
  return starting;
}

int dir_entry_present(int entry) {
  uint32_t dir_entry=page_directory[entry];
  return dir_entry&1;
}

void* virt_to_phys(void* virt_addr_ptr) {
  uint32_t virt_addr=(uint32_t)virt_addr_ptr;
  int dir_num=(virt_addr&0xFFC00000)>>22;
  int table_num=(virt_addr&0x3FF000)>>12;
  int offset=(virt_addr&0xFFF);
  uint32_t table_entry=page_tables[(dir_num*1024)+table_num];
  table_entry=table_entry*0xFFFFF000;
  return (void*)(table_entry+offset);

}

void initialize_paging() {
  alloc_kern_pages(NUM_KERN_DIRS*1024,1);
  load_page_directory(virt_to_phys(page_directory));
}
