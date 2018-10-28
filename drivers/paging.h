#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

void set_directory_entry(int entry, int table_no, char usr, char wren, char p);
void set_table_entry(int dir_entry, int page, uint32_t base_addr, char usr, char wren, char p);
int dir_entry_present(int entry);
void init_paging();

#endif
