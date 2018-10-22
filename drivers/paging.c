#include <stdint.h>
#include "paging_helpers.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));
// uint32_t page_table[1024] __attribute__((aligned(4096)));


void init_paging() {
  unsigned int i;
  for(i = 0; i < 1024; i++) {
      // This sets the following flags to the pages:
      //   Supervisor: Only kernel-mode can access them
      //   Write Enabled: It can be both read from and written to
      //   Not Present: The page table is not present
      page_directory[i] = 0x00000002;
  }
  for(i = 0; i < 1024; i++) {
      page_tables[1][i] = (i * 0x1000) | 7;
  }
  page_directory[0] = ((uint32_t)page_tables[1]) | 7;
  load_page_directory(page_directory);
  enable_paging();
}
