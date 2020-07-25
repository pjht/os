#include "idt.h"
#include <stdint.h>

/* Segment selectors */
#define KERNEL_CS 0x08

/* How every interrupt gate (handler) is defined */
typedef struct {
    uint16_t low_offset; /* Low 16 bits of handler function address */
    uint16_t sel; /* Kernel segment selector */
    uint8_t always0;
    /* First byte
     * Bit 7: "Interrupt is present"
     * Bits 6-5: Privilege level of caller (0=kernel..3=user)
     * Bit 4: Set to 0 for interrupt gates
     * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
    uint8_t flags;
    uint16_t middle_offset; /* Middle 16 bits of handler function address */
    uint32_t high_offset; /* High 32 bits of handler function address */
    uint32_t also0;
} __attribute__((packed)) idt_gate_t ;

/* A pointer to the array of interrupt handlers.
 * Assembly instruction 'lidt' will read it */
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

static idt_gate_t idt[IDT_ENTRIES];
static idt_register_t idt_reg;

#define LOW_16(address) (uint16_t)((address) & 0xFFFF)
#define middle_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)
#define high_32(address) (uint32_t)(((address) >> 32) & 0xFFFFFFFF)


void idt_set_gate(int n,uint64_t handler) {
    idt[n].low_offset=LOW_16(handler);
    idt[n].sel=KERNEL_CS;
    idt[n].always0=0;
    idt[n].flags=0xEE;
    idt[n].middle_offset=middle_16(handler);
    idt[n].high_offset=high_32(handler);
    idt[n].also0=0;
}

void load_idt() {
    idt_reg.base=(uint64_t) &idt;
    idt_reg.limit=IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    /* Don't make the mistake of loading &idt -- always load &idt_reg */
    asm volatile("lidtq (%0)":: "r" (&idt_reg));
}
