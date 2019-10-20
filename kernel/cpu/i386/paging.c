#include <stdint.h>
#include <stdlib.h>
#include "paging_helpers.h"
#include "paging.h"
#include "pmem.h"
#include "../..//vga_err.h"
#include <klog.h>

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t kern_page_tables[NUM_KERN_DIRS*1024] __attribute__((aligned(4096)));
static uint32_t kstack_page_tables[32*1024] __attribute__((aligned(4096)));
static uint32_t kmalloc_page_tables[4*1024] __attribute__((aligned(4096)));
static uint32_t smap_page_tables[2048] __attribute__((aligned(4096)));
static uint32_t* smap=(uint32_t*)0xFF800000;

static char is_page_present(int page) {
   int table=page>>10;
   page=page&0x3FF;
   if ((smap[table]&0x1)==0) {
     return 0;
   }
   smap_page_tables[table+1]=(smap[table]&0xFFFFFC00)|0x3;
   return smap[(1024+(1024*table))+page]&0x1;
}

void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr) {
  uint32_t virt_addr=(uint32_t)virt_addr_ptr;
  uint32_t phys_addr=(uint32_t)phys_addr_ptr;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<=num_pages;i++) {
    if (!(smap[dir_entry]&0x1)) {
      int flags=1;
      flags=flags|((wr&1)<<1);
      flags=flags|((usr&1)<<2);
      smap[dir_entry]=(uint32_t)pmem_alloc(1)|flags;
    }
    smap_page_tables[dir_entry+1]=(smap[dir_entry]&0xFFFFFC00)|0x3;
    int flags=1;
    flags=flags|((wr&1)<<1);
    flags=flags|((usr&1)<<2);
    smap[(1024+(1024*dir_entry))+table_entry]=phys_addr|flags;
    table_entry++;
    if (table_entry==1024) {
      table_entry=0;
      dir_entry++;
    }
    phys_addr+=0x1000;
  }
}

void map_kstack(uint32_t pid) {
  if (!(kstack_page_tables[pid]&0x1)) {
    kstack_page_tables[pid]=(uint32_t)pmem_alloc(1)|0x3;
  }
}

void* find_free_pages(int num_pages) {
  uint32_t bmap_index;
  uint32_t remaining_blks;
  for(uint32_t i=1;i<131072;i++) {
    char got_0=0;
    remaining_blks=num_pages;
    uint32_t old_j;
    for (uint32_t j=i*8;;j++) {
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
  if ((smap[dir_idx]&0x1)==0) {
    return 0;
  }
  smap_page_tables[dir_idx+1]=(smap[dir_idx]&0xFFFFFC00)|0x3;
  return (void*)((smap[(1024+(1024*dir_idx))+tbl_idx]&0xFFFFFC00)+offset);
}


void alloc_pages_virt(int num_pages,void* addr) {
  void* phys_addr=pmem_alloc(num_pages);
  map_pages(addr,phys_addr,num_pages,1,1);
}

void invl_page(void* addr) {
  asm volatile("invlpg (%0)"::"r"(addr):"memory");
}

void* paging_new_address_space() {
  uint32_t cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
  void* dir=pmem_alloc(1);
  smap_page_tables[0]=((uint32_t)dir)|0x3;
  invl_page(smap);
  for (uint32_t i=0;i<1024;i++) {
    smap[i]=page_directory[i];
  }
  smap_page_tables[0]=cr3|0x3;
  invl_page(smap);
  return dir;
}

void load_address_space(uint32_t cr3) {
  load_smap(cr3);
  load_page_directory((uint32_t*)cr3);
}

void load_smap(uint32_t cr3) {
  smap_page_tables[0]=cr3|0x3;
  invl_page(&smap[0]);
  for (uint32_t i=1;i<2048;i++) {
    invl_page(&smap[i*1024]);
    smap_page_tables[i]=0;
  }
}

void unmap_pages(void* start_virt,uint32_t num_pages) {
  uint32_t virt_addr=(uint32_t)start_virt;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (uint32_t i=0;i<=num_pages;i++) {
    if (smap[dir_entry]&0x1) {
      smap_page_tables[dir_entry+1]=(smap[dir_entry]&0xFFFFFC00)|0x3;
      smap[(1024+(1024*dir_entry))+table_entry]=0;
      invl_page(start_virt+(i*1024));
      table_entry++;
      if (table_entry==1024) {
        dir_entry++;
        table_entry=0;
      }
    }
  }
}

char make_protector(int page) {
  int table=page>>10;
  if (is_page_present(page)) return 0;
  page=page&0x3FF;
  smap_page_tables[table+1]=(smap[table]&0xFFFFFC00)|0x3;
  uint32_t page_val=smap[(1024+(1024*table))+page];
  page_val=page_val&(~0x6);
  page_val=page_val|0x800;
  smap[(1024+(1024*table))+page]=page_val;
  return 1;
}

char is_in_protector(uint32_t* addr) {
  int page=((uint32_t)addr)>>12;
  if (is_page_present(page)) return 0;
  int table=page>>10;
  page=page&0x3FF;
  smap_page_tables[table+1]=(smap[table]&0xFFFFFC00)|0x3;
  return smap[(1024+(1024*table))+page]&0x800;
  return 1;
}

void paging_init() {
  for (uint32_t i=0;i<NUM_KERN_DIRS*1024;i++) {
    kern_page_tables[i]=(i<<12)|0x3;
  }
  for (uint32_t i=0;i<32*1024;i++) {
    kstack_page_tables[i]=0;
  }
  for (uint32_t i=0;i<4*1024;i++) {
    kmalloc_page_tables[i]=(uint32_t)pmem_alloc(1)|0x3;
  }
  smap_page_tables[0]=(((uint32_t)&(page_directory))-0xC0000000)|0x3;
  for (uint32_t i=1;i<2048;i++) {
    smap_page_tables[i]=0;
  }
  for (uint32_t i=0;i<NUM_KERN_DIRS;i++) {
    uint32_t entry_virt=(uint32_t)&(kern_page_tables[i*1024]);
    page_directory[i+768]=(entry_virt-0xC0000000)|0x3;
  }
  page_directory[985]=(uint32_t)(pmem_alloc(1024))|0x83;
  for (uint32_t i=0;i<32;i++) {
    uint32_t entry_virt=(uint32_t)&(kstack_page_tables[i*1024]);
    page_directory[i+986]=(entry_virt-0xC0000000)|0x3;
  }
  for (uint32_t i=0;i<4;i++) {
    uint32_t entry_virt=(uint32_t)&(kmalloc_page_tables[i*1024]);
    page_directory[i+1018]=(entry_virt-0xC0000000)|0x3;
  }
  // page_directory[1018,1021]=(((uint32_t)kmalloc_page_tables)-0xC0000000)|0x3;
  for (uint32_t i=0;i<2;i++) {
    uint32_t entry_virt=(uint32_t)&(smap_page_tables[i*1024]);
    page_directory[i+1022]=(entry_virt-0xC0000000)|0x3;
  }
  load_page_directory((uint32_t*)((uint32_t)page_directory-0xC0000000));
}
