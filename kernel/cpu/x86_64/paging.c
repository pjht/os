#include "paging.h"
#include "pmem.h"
#include <stdint.h>

static uint64_t pml4[512] __attribute__((aligned(4096)));
static uint64_t pdpt[512] __attribute__((aligned(4096)));
static uint64_t page_directory[512] __attribute__((aligned(4096)));
uint64_t* curr_structs=(uint64_t*)0xFFFFFF8000000000;


uint64_t get_pml4_entry(int entry) {
  return ((uint64_t*)(((char*)curr_structs)+0x7FFFFFF000))[entry];
}

uint64_t get_pdpt_entry(int pml4_entry,int entry) {
  return ((uint64_t*)(((char*)curr_structs)+0x7FFFE00000))[(pml4_entry<<9)+entry];
}


uint64_t get_page_dir_entry(int pml4_entry,int pdpt_entry, int entry) {
  return ((uint64_t*)(((char*)curr_structs)+0x7FC0000000))[(pml4_entry<<18)+(pdpt_entry<<9)+entry];
}

uint64_t get_page_table_entry(int pml4_entry,int pdpt_entry, int pdir_entry, int entry) {
  return ((uint64_t*)(((char*)curr_structs)+0x7000000000))[(pml4_entry<<27)+(pdpt_entry<<18)+(pdir_entry<<9)+entry];
}

void set_pml4_entry(int entry,uint64_t mapping) {
  ((uint64_t*)(((char*)curr_structs)+0x7FFFFFF000))[entry]=mapping;
}

void set_pdpt_entry(int pml4_entry,int entry,uint64_t mapping) {
  ((uint64_t*)(((char*)curr_structs)+0x7FFFE00000))[(pml4_entry<<9)+entry]=mapping;
}

void set_page_dir_entry(int pml4_entry,int pdpt_entry, int entry,uint64_t mapping) {
  ((uint64_t*)(((char*)curr_structs)+0x7FC0000000))[(pml4_entry<<18)+(pdpt_entry<<9)+entry]=mapping;
}

void set_page_table_entry(int pml4_entry,int pdpt_entry, int pdir_entry, int entry,uint64_t mapping) {
  ((uint64_t*)(((char*)curr_structs)+0x7000000000))[(pml4_entry<<27)+(pdpt_entry<<18)+(pdir_entry<<9)+entry]=mapping;
}

uint32_t find_free_page() {
  for (int a=0;a<512;a++) {
    if (get_pml4_entry(a)&0x1) {
      for (int b=0;b<512;b++) {
        if (get_pdpt_entry(a,b)&0x1) {
          for (int c=0;c<512;c++) {
            if (get_page_dir_entry(a,b,c)&0x1) {
              for (int d=0;d<512;d++) {
                if (!(get_page_table_entry(a,b,c,d)&0x1)) {
                  return (a<<27)+(b<<18)+(c<<9)+d;
                }
              } // end for d
            } else {
              uint64_t addr=pmem_alloc(1);
              set_page_dir_entry(a,b,c,addr|0x7);
              return (a<<27)+(b<<18)+(c<<9);
            } // end if abc
          } // end for c
        } else {
          uint64_t addr=pmem_alloc(1);
          set_pdpt_entry(a,b,addr|0x7);
          addr=pmem_alloc(1);
          set_page_dir_entry(a,b,0,addr|0x7);
          return (a<<27)+(b<<18);
        } // end if ab
      } // end for b
    } else {
      uint64_t addr=pmem_alloc(1);
      set_pml4_entry(a,addr|0x7);
      addr=pmem_alloc(1);
      set_pdpt_entry(a,0,addr|0x7);
      addr=pmem_alloc(1);
      set_page_dir_entry(a,0,0,addr|0x7);
      return a<<27;
    } //end if a
  } // end for a
  return 0;
}

void paging_init() {
  pml4[0]=(((uint64_t)&pdpt)-0xffff800000000000)|0x7;
  pml4[256]=(((uint64_t)&pdpt)-0xffff800000000000)|0x7;
  pml4[511]=(((uint64_t)&pml4)-0xffff800000000000)|0x7;
  pdpt[0]=(((uint64_t)&page_directory)-0xffff800000000000)|0x7;
  for (int i=0;i<NUM_KERN_FRAMES/1024;i++) {
    page_directory[i]=(i*0x200000)|0x87;
  }
  asm volatile("mov %0,%%cr3"::"r"((uint64_t)pml4-0xffff800000000000));
}

void check_gets() {
  char str[256];
  hex_to_ascii(find_free_page(),str);
  vga_write_string(str);
  vga_write_string("\n");
  str[0]='\0';
  hex_to_ascii(find_free_page(),str);
  vga_write_string(str);
  vga_write_string("\n");
}
