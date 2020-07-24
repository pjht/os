#include "../../pmem.h"
#include "../../vga_err.h"
#include "../halt.h"
#include "../paging.h"
#include "arch_consts.h"
#include "paging_helpers.h"
#include <klog.h>
#include <stdint.h>
#include <stdlib.h>

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t kern_page_tables[NUM_KERN_FRAMES] __attribute__((aligned(4096)));
static uint32_t kstack_page_tables[218*1024] __attribute__((aligned(4096)));
static uint32_t kmalloc_page_tables[4*1024] __attribute__((aligned(4096)));
static uint32_t* pagdirmap=(uint32_t*)0xFFFFF000;
static uint32_t* pagtblmap=(uint32_t*)0xFFC00000;
static char is_page_present(size_t page) {
   int table=page>>10;
   page=page&0x3FF;
   if ((pagdirmap[table]&0x1)==0) {
     return 0;
   }
   return pagtblmap[page+1024*table]&0x1;
}

void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr) {
  uint32_t virt_addr=(uint32_t)virt_addr_ptr;
  uint32_t phys_addr=(uint32_t)phys_addr_ptr;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<num_pages;i++) {
    if (!(pagdirmap[dir_entry]&0x1)) {
      int flags=1;
      flags=flags|((wr&1)<<1);
      flags=flags|((usr&1)<<2);
      pagdirmap[dir_entry]=(uint32_t)pmem_alloc(1)|flags;
    }
    int flags=1;
    flags=flags|((wr&1)<<1);
    flags=flags|((usr&1)<<2);
    pagtblmap[table_entry+1024*dir_entry]=phys_addr|flags;
    table_entry++;
    if (table_entry==1024) {
      table_entry=0;
      dir_entry++;
    }
    phys_addr+=0x1000;
  }
}

void* find_free_pages(int num_pages) {
  size_t bmap_index;
  size_t remaining_blks;
  for(size_t i=1;i<131072;i++) {
    char got_0=0;
    remaining_blks=num_pages;
    size_t old_j;
    for (size_t j=i*8;;j++) {
      char bit=is_page_present(j);
      if (got_0) {
        if (bit) {
          if (remaining_blks==0) {
              bmap_index=old_j;
              break;
          } else {
            i+=j/8;
            i--;
            break;
          }
        } else {
          remaining_blks--;
        }
      } else {
        if (!bit) {
          got_0=1;
          old_j=j;
          remaining_blks--;
        }
      }
      if (remaining_blks==0) {
        bmap_index=old_j;
        break;
      }
    }
    if (remaining_blks==0) {
      break;
    }
  }
  if (remaining_blks!=0) {
    vga_write_string("[PANIC] Out of memory");
    halt();
  }
  return (void*)(bmap_index<<12);
}

void* alloc_pages(int num_pages) {
  void* phys_addr=pmem_alloc(num_pages);
  void* addr=find_free_pages(num_pages);
  map_pages(addr,phys_addr,num_pages,1,1);
  return addr;
}

void* virt_to_phys(void* virt_addr_arg) {
  uint32_t virt_addr=(uint32_t)virt_addr_arg;
  int offset=virt_addr&0x3FF;
  virt_addr=virt_addr&0xFFFFFC00;
  if (!is_page_present(virt_addr>>12)) return NULL;
  int dir_idx=(virt_addr&0xFFC00000)>>22;
  int tbl_idx=(virt_addr&0x3FFC00)>>12;
  if ((pagdirmap[dir_idx]&0x1)==0) {
    return 0;
  }
  return (void*)((pagtblmap[tbl_idx+1024*dir_idx]&0xFFFFFC00)+offset);
}


void alloc_pages_virt(int num_pages,void* addr) {
  void* phys_addr=pmem_alloc(num_pages);
  map_pages(addr,phys_addr,num_pages,1,1);
}

void invl_page(void* addr) {
  asm volatile("invlpg (%0)"::"r"(addr):"memory");
}

void* paging_new_address_space() {
  void* dir=pmem_alloc(1);
  uint32_t* freepg=find_free_pages(1);
  map_pages(freepg,dir,1,0,1);
  for (size_t i=0;i<1024;i++) {
    freepg[i]=page_directory[i];
  }
  freepg[1023]=((uint32_t)dir)|0x3;
  unmap_pages(freepg,1);
  return dir;
}

void load_address_space(void* cr3) {
  load_page_directory((uint32_t*)cr3);
}

void unmap_pages(void* start_virt,int num_pages) {
  uint32_t virt_addr=(uint32_t)start_virt;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<=num_pages;i++) {
    if (pagtblmap[dir_entry]&0x1) {
      pagtblmap[table_entry+1024*dir_entry]=0;
      invl_page(start_virt+(i*1024));
      table_entry++;
      if (table_entry==1024) {
        dir_entry++;
        table_entry=0;
      }
    }
  }
}

void paging_init() {
  for (size_t i=0;i<NUM_KERN_FRAMES;i++) {
    kern_page_tables[i]=(i<<12)|0x3;
  }
  for (size_t i=0;i<218*1024;i++) {
    kstack_page_tables[i]=0;
  }
  for (size_t i=0;i<4*1024;i++) {
    kmalloc_page_tables[i]=(uint32_t)pmem_alloc(1)|0x3;
  }
  for (size_t i=0;i<NUM_KERN_FRAMES/1024;i++) {
    uint32_t entry_virt=(uint32_t)&(kern_page_tables[i*1024]);
    page_directory[i+768]=(entry_virt-0xC0000000)|0x3;
  }
  page_directory[985]=(uint32_t)(pmem_alloc(1024))|0x83;
  for (size_t i=0;i<4;i++) {
    uint32_t entry_virt=(uint32_t)&(kmalloc_page_tables[i*1024]);
    page_directory[i+1018]=(entry_virt-0xC0000000)|0x3;
  }
  page_directory[1023]=((uint32_t)page_directory-0xC0000000)|0x3;
  load_page_directory((uint32_t*)((uint32_t)page_directory-0xC0000000));
}

void* get_cr3() {
  void* cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
  return cr3;
}
