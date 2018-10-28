#include <stdint.h>
#include "paging_helpers.h"

uint32_t *page_directory; //[1024] __attribute__((aligned(4096)));
uint32_t *page_tables; //[1048576] __attribute__((aligned(4096)));

#define INIT_DIR_ENTRIES 2

void set_directory_entry(int entry, int table_no, char usr, char wren, char p) {
    int flags=p&1;
    flags=flags|((wren&1)<<1);
    flags=flags|((usr&1)<<2);
    page_directory[entry]=((uint32_t)&(page_tables[table_no]))|flags;
}

void set_table_entry(int dir_entry, int page, uint32_t base_addr, char usr, char wren, char p) {
    int flags=p&1;
    flags=flags|((wren&1)<<1);
    flags=flags|((usr&1)<<2);
    page_tables[(dir_entry*1024)+page]=(base_addr)|flags;
}

int dir_entry_present(int entry) {
  uint32_t dir_entry=page_directory[entry];
  return dir_entry&1;
}

void init_paging() {
  page_directory=0x100000;
  page_tables=0x410000;
  for(int i=0;i<1024;i++) {
    set_directory_entry(i,0,0,0,0);
  }
  char s[20];
  for(int dir=0;dir<INIT_DIR_ENTRIES;dir++) {
    for(int page=0;page<1024;page++) {
      set_table_entry(dir,page,((dir*1024)+page)*0x1000,1,1,1);
    }
  }
  for(int i=0;i<INIT_DIR_ENTRIES;i++) {
    set_directory_entry(i,i,1,1,1);
  }
  set_directory_entry(0,0,1,1,1);
  load_page_directory(page_directory);
  enable_paging();
}
