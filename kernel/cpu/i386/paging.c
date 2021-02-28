/**
 * \file 
*/

#include "../../pmem.h"
#include "../../vga_err.h"
#include "../halt.h"
#include "../paging.h"
#include "arch_consts.h"
#include <stdint.h>
#include <stdlib.h>

/**
 * Represents an entry in a page table/directory. 
 * \note Privlege bits in the page directory and page table entries for a page are ANDed together, so the most restrictive privlege between the page directory and the page table wins.
*/
typedef struct {
  int pres:1; //!< Whether the page is present
  int wr:1; //!< Whether the page is writeable
  int usr:1; //!< Whether the page is accessible by user mode
  int cachetype:1; //!< Cache type for the page. Write-through caching when 1, write-back caching when 0.
  int cachedisable:1; //!< Whether caching is disabled
  int accessed:1; //!< Whether the page has been accessed
  int dirty:1; //!< Whether the page is dirty (has been written to)
  int sz:1; //!< Page size
  int osavail:4; //!< Availible for OS use
  int pgno:20; //!< Physical page number this page maps to
} pg_struct_entry;

static pg_struct_entry page_directory[1024] __attribute__((aligned(4096))); //!< The kernel process's page directory
static pg_struct_entry kern_page_tables[NUM_KERN_FRAMES] __attribute__((aligned(4096))); //!< The page tables where the kernel binary is mapped in
static pg_struct_entry kstack_page_tables[218*1024] __attribute__((aligned(4096))); //!< Page tables for thread kernel stacks
static pg_struct_entry kmalloc_page_tables[4*1024] __attribute__((aligned(4096))); //!< Page tables for the kmalloc heap
static pg_struct_entry* pagdirmap=(pg_struct_entry*)0xFFFFF000; //!< Pointer to the page directory entries in the recursive mapping
static pg_struct_entry* page_table_map=(pg_struct_entry*)0xFFC00000; //!< Pointer to the page table entries in the recursive mapping

char is_page_present(size_t page) {
   int table=page>>10;
   page=page&0x3FF;
   if (!pagdirmap[table].pres) {
     return 0;
   }
   return page_table_map[page+1024*table].pres;
}

void map_pages(void* virt_addr_ptr,void* phys_addr_ptr,int num_pages,char usr,char wr) {
  uint32_t virt_addr=(uint32_t)virt_addr_ptr;
  uint32_t phys_addr=(uint32_t)phys_addr_ptr;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<num_pages;i++) {
    if (!pagdirmap[dir_entry].pres) {
      pg_struct_entry* entry=&pagdirmap[dir_entry];
      entry->pgno=(uint32_t)pmem_alloc(1)>>12;
      entry->pres=1;
      entry->usr=usr;
      entry->wr=wr;
    }
    pg_struct_entry* entry=&page_table_map[table_entry+1024*dir_entry];
    if (phys_addr_ptr==NULL) {
      phys_addr=(uint32_t)pmem_alloc(1);
    }
    entry->pgno=phys_addr>>12;
    entry->pres=1;
    entry->usr=usr;
    entry->wr=wr;
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
  void* addr=find_free_pages(num_pages);
  map_pages(addr,NULL,num_pages,1,1);
  return addr;
}

void* virt_to_phys(void* virt_addr_arg) {
  uint32_t virt_addr=(uint32_t)virt_addr_arg;
  int offset=virt_addr&0x3FF;
  virt_addr=virt_addr&0xFFFFFC00;
  if (!is_page_present(virt_addr>>12)) return NULL;
  int dir_idx=(virt_addr&0xFFC00000)>>22;
  int tbl_idx=(virt_addr&0x3FFC00)>>12;
  if (!pagdirmap[dir_idx].pres) {
    return 0;
  }
  return (void*)(((page_table_map[tbl_idx+1024*dir_idx].pgno)<<12)+offset);
}


void alloc_pages_virt(int num_pages,void* addr) {
  map_pages(addr,NULL,num_pages,1,1);
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
  pg_struct_entry* freepg=find_free_pages(1);
  map_pages(freepg,dir,1,0,1);
  for (size_t i=0;i<1024;i++) {
    freepg[i]=page_directory[i];
  }
  pg_struct_entry* entry=&freepg[1023];
  entry->pres=1;
  entry->wr=1;
  entry->pgno=(uint32_t)dir>>12;
  unmap_pages(freepg,1,0);
  return dir;
}

void load_address_space(void* address_space) {
  asm volatile("movl %0, %%eax; movl %%eax, %%cr3;":"=m"(address_space)::"%eax");
}

void unmap_pages(void* start_virt,int num_pages,int free_phys) {
  uint32_t virt_addr=(uint32_t)start_virt;
  int dir_entry=(virt_addr&0xFFC00000)>>22;
  int table_entry=(virt_addr&0x3FF000)>>12;
  for (int i=0;i<=num_pages;i++) {
    if (page_table_map[dir_entry].pres) {
      pg_struct_entry* entry=&page_table_map[table_entry+1024*dir_entry];
      entry->pres=0;
      if (free_phys) {
        pmem_free((void*)(entry->pgno<<12),1);
      }
      invl_page(start_virt+(i*4096));
      table_entry++;
      if (table_entry==1024) {
        dir_entry++;
        table_entry=0;
      }
    }
  }
}

extern int idt;

/**
 * Makes the IDT readonly
*/
void paging_readonly_idt() {
  void* idt_addr=&idt;
  void* idt_phys=virt_to_phys(idt_addr);
  map_pages(idt_addr,idt_phys,1,0,0);
  invl_page(idt_addr);
}

void paging_init() {
  for (size_t i=0;i<NUM_KERN_FRAMES;i++) {
    pg_struct_entry* entry=&kern_page_tables[i];
    entry->pres=1;
    entry->wr=1;
    entry->pgno=i;
  }
  for (size_t i=0;i<218*1024;i++) {
    pg_struct_entry* entry=&kstack_page_tables[i];
    entry->pres=0;
  }
  for (size_t i=0;i<4*1024;i++) {
    pg_struct_entry* entry=&kmalloc_page_tables[i];
    entry->pres=1;
    entry->wr=1;
    entry->pgno=(uint32_t)pmem_alloc(1)>>12;
  }
  for (size_t i=0;i<NUM_KERN_FRAMES/1024;i++) {
    uint32_t entry_virt=(uint32_t)&(kern_page_tables[i*1024]);
    pg_struct_entry* entry=&page_directory[i+768];
    entry->pres=1;
    entry->wr=1;
    entry->pgno=((uint32_t)entry_virt-0xC0000000)>>12;
  }
  for (size_t i=0;i<4;i++) {
    uint32_t entry_virt=(uint32_t)&(kmalloc_page_tables[i*1024]);
    pg_struct_entry* entry=&page_directory[i+1018];
    entry->pres=1;
    entry->wr=1;
    entry->pgno=((uint32_t)entry_virt-0xC0000000)>>12;
  }
  pg_struct_entry* entry=&page_directory[1023];
  entry->pres=1;
  entry->wr=1;
  entry->pgno=((uint32_t)page_directory-0xC0000000)>>12;
  load_address_space((uint32_t*)((uint32_t)page_directory-0xC0000000));
}

void* get_address_space() {
  void* address_space;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(address_space)::"%eax");
  return address_space;
}

void dealloc_pages(int num_pages,void* addr) {
  pmem_free((void*)((uint32_t)virt_to_phys(addr)>>12),num_pages);
  unmap_pages(addr,num_pages,1);
}
