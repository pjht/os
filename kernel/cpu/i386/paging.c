/**
 * \file 
*/

#include "../../pmem.h"
#include "../../vga_err.h"
#include "../halt.h"
#include "../paging.h"
#include "arch_consts.h"
#include <klog.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * \page pg_struct_entry Format of a paging structure entry
 * The format of a page table/directiry entry is as following: <br>
 * Bits 31-11 is the physical frame number the entry points to. <br>
 * Bits 11-9 are availible for use by the OS. <br>
 * Bit 8 is ignored. <br>
 * Bit 7 is the page size in page directories, and must be 0 in page tables. If set to 1 in a page directory, it indicates 4MB pages. <br>
 * Bit 6 is the dirty bit in page tables, and must be 0 in page directories. In page tabes, it is set to 1 by the CPU when the page is written to. <br>
 * Bit 5 will be set to 1 by the CPU when the page is accessed. <br>
 * Bit 4 indicates whether the page has it's cache disabled. <br>
 * Bit 3 indictates whether write-through caching (when it is 1), or write-back caching, (when it is 0) is enabled. <br>
 * Bit 2 indictaes whether user mode code can access the page. <br>
 * Bit 1 indicates whether the page is writable. <br>
 * Bit 0 indicates whether the entry is present. If it is 0, the CU ignores the other 31 bits of the entry. <br>
 * Privlege bits in the entries are ANDed together, so the most restrictive privlege between the page directory and the page table wins.
*/

static uint32_t page_directory[1024] __attribute__((aligned(4096))); //!< The kernel process's page directory
static uint32_t kern_page_tables[NUM_KERN_FRAMES] __attribute__((aligned(4096))); //!< The page tables where the kernel binary is mapped in
static uint32_t kstack_page_tables[218*1024] __attribute__((aligned(4096))); //!< Page tables for thread kernel stacks
static uint32_t kmalloc_page_tables[4*1024] __attribute__((aligned(4096))); //!< Page tables for the kmalloc heap
static uint32_t* pagdirmap=(uint32_t*)0xFFFFF000; //!< Pointer to the page directory entries in the recursive mapping
static uint32_t* page_table_map=(uint32_t*)0xFFC00000; //!< Pointer to the page table entries in the recursive mapping
/**
 * Checks whether a page is present
 * \param page The page number to check
 * \return Whether the page is present
*/
static char is_page_present(size_t page) {
   int table=page>>10;
   page=page&0x3FF;
   if ((pagdirmap[table]&0x1)==0) {
     return 0;
   }
   return page_table_map[page+1024*table]&0x1;
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
    page_table_map[table_entry+1024*dir_entry]=phys_addr|flags;
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
  return (void*)((page_table_map[tbl_idx+1024*dir_idx]&0xFFFFFC00)+offset);
}


void alloc_pages_virt(int num_pages,void* addr) {
  void* phys_addr=pmem_alloc(num_pages);
  map_pages(addr,phys_addr,num_pages,1,1);
}

/**
 * Invalidates a page in the TLB,
 * \param addr The address of the page to invalidate.
*/
static void invl_page(void* addr) {
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
  asm volatile("movl %0, %%eax; movl %%eax, %%cr3;":"=m"(cr3)::"%eax");
}

void unmap_pages(void* start_virt,int num_pages) {
  uint32_t virt_addr=(uint32_t)start_virt;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<=num_pages;i++) {
    if (page_table_map[dir_entry]&0x1) {
      page_table_map[table_entry+1024*dir_entry]=0;
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
  load_address_space((uint32_t*)((uint32_t)page_directory-0xC0000000));
}

void* get_cr3() {
  void* cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
  return cr3;
}
