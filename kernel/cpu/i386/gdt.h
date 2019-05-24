#ifndef GDT_H
#define GDT_H

void gdt_init();
void tss_stack_reset();
void allow_all_ports();
void block_all_ports();
#endif
