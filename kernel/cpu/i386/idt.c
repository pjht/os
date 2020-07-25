/**
 * \file 
*/

#include "idt.h"
#include <stdint.h>

#define KERNEL_CS 0x08 //!< Kernel code segemnt selector

/**
 * Defines an interrupt gate
*/
typedef struct {
  uint16_t low_offset; //!< Lower 16 bits of handler function address
  uint16_t sel; //!< Kernel segment selector
  uint8_t always0; //!< Must be 0.
  uint8_t flags; /**<
                    Flags byte. Gives info about the descriptor
                    * Bit 7: Present. Must be 1 for all valid selectors.
                    * Bits 6-5: Privilege. Contains the minimum ring level for the caller. 0 for kernel mode, 3 for user mode. 
                    * Bit 4: Set to 0 for interrupt gates.
                    * Bits 3-0: 1110 = "32 bit interrupt gate".
                  */
  uint16_t high_offset; //!< Higher 16 bits of handler function address
} __attribute__((packed)) idt_gate_t;

/**
 * Pointed to by the IDTR to tell the processor the IDT's size and address.
*/
typedef struct {
  uint16_t limit; //!< Size of the IDT.
  idt_gate_t* base;  //!< Address of the IDT.
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256 //!< Number of entries in the IDT

static idt_gate_t idt[IDT_ENTRIES]; //!< The IDT
static idt_register_t idt_reg; //!< The value to load into the IDTR

#define low_16(address) (uint16_t)((address) & 0xFFFF) //!< Macro to get the low 16 bits of an address
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF) //!< Macro to get the high 16 bits of an address


void idt_set_gate(int n,uint32_t handler) {
  idt[n].low_offset=low_16(handler);
  idt[n].sel=0x08;
  idt[n].always0=0;
  idt[n].flags=0xEE;
  idt[n].high_offset=high_16(handler);
}

void load_idt() {
  idt_reg.base=&idt[0];
  idt_reg.limit=IDT_ENTRIES * sizeof(idt_gate_t) - 1;
  /* Don't make the mistake of loading &idt -- always load &idt_reg */
  asm volatile("lidtl (%0)":: "r" (&idt_reg));
}
