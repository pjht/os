/**
 * \file 
*/

#ifndef IDT_H
#define IDT_H


#include <stdint.h>

/**
 * Sets an IDT gate.
 * \param n the IDT gate to set
 * \param handler the handler for the gate.
*/
void idt_set_gate(int n,uint32_t handler);
/**
 * Loads the IDT
*/
void load_idt();

#endif
