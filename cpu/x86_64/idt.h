#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Functions implemented in idt.c */
void idt_set_gate(int n,uint64_t handler);
void load_idt();

#endif
