#include <stdint.h>
#include <string.h>
#include "seg_upd.h"
#define NUM_ENTRIES 6

extern uint32_t int_stack_top;

typedef struct {
  uint16_t limit_low16;
  uint16_t base_low16;
  uint8_t base_mid8;
  uint8_t access;
  uint8_t limit_flags;
  uint8_t base_high8;
} __attribute__((packed)) gdt_entry;

typedef struct {
  uint16_t size;
  gdt_entry* address;
} __attribute__((packed)) gdt_description;

typedef struct {
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // Unused...
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         // The value to load into ES when we change to kernel mode.
   uint32_t cs;         // The value to load into CS when we change to kernel mode.
   uint32_t ss;         // The value to load into SS when we change to kernel mode.
   uint32_t ds;         // The value to load into DS when we change to kernel mode.
   uint32_t fs;         // The value to load into FS when we change to kernel mode.
   uint32_t gs;         // The value to load into GS when we change to kernel mode.
   uint32_t ldt;        // Unused...
   uint16_t trap;
   uint16_t iomap_base;
   char iopb[8192]; // IO port bitmap
   uint8_t set_ff;
} __attribute__((packed)) tss_entry;

static gdt_entry gdt[NUM_ENTRIES];
static gdt_description gdt_desc;
static tss_entry tss;

void tss_stack_reset() {
  tss.esp0=int_stack_top+0xC0000000;
}

static void set_entry(int i,uint32_t base,uint32_t limit,uint8_t access) {
  gdt[i].limit_low16=limit&0xFFFF;
  gdt[i].base_low16=base&0xFFFFF;
  gdt[i].base_mid8=(base&0xFF0000)>>16;
  gdt[i].access=access;
  uint8_t limit_high4=(limit&0xF0000)>>16;
  gdt[i].limit_flags=0xC0|limit_high4;
  gdt[i].base_high8=(base&0xFF000000)>>24;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
  // Firstly, let's compute the base and limit of our entry into the GDT.
  uint32_t base = (uint32_t) &tss;
  uint32_t limit = base + sizeof(tss_entry);

  // Now, add our TSS descriptor's address to the GDT.
  gdt[num].limit_low16=limit&0xFFFF;
  gdt[num].base_low16=base&0xFFFFF;
  gdt[num].base_mid8=(base&0xFF0000)>>16;
  gdt[num].access=0xe9;
  gdt[num].limit_flags=(limit&0xF0000)>>16;
  gdt[num].base_high8=(base&0xFF000000)>>24;

  // Ensure the descriptor is initially zero.
  memset((void*)&tss,0,sizeof(tss));
  tss.ss0  = ss0;  // Set the kernel stack segment.
  tss.esp0 = esp0; // Set the kernel stack pointer.

  //Set the last byte to 0xFF (End marker for IOPB)
  tss.set_ff=0xFF;

  // Now, set the offset for the IOPB
  // (All ports are already OK from the zeroing)
  tss.iomap_base=104;


  // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
  // segments should be loaded when the processor switches to kernel mode. Therefore
  // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
  // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
  // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
  // to switch to kernel mode from ring 3.
  tss.cs = 0x0b;
  tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

void allow_all_ports() {
  for (int i=0;i<8192;i++) {
    tss.iopb[i]=0;
  }
}


void block_all_ports() {
  for (int i=0;i<8192;i++) {
    tss.iopb[i]=0xFF;
  }
}


void gdt_init() {
  set_entry(0,0,0,0);
  set_entry(1,0,0xFFFFF,0x9A);
  set_entry(2,0,0xFFFFF,0x92);
  set_entry(3,0,0xFFFFF,0xFA);
  set_entry(4,0,0xFFFFF,0xF2);
  write_tss(5,0x10,int_stack_top+0xC0000000);
  gdt_desc.size=(sizeof(gdt_entry)*NUM_ENTRIES)-1;
  gdt_desc.address=gdt;
  asm volatile("lgdt (%%eax)"::"a"((uint32_t)&gdt_desc));
  seg_upd();
  asm volatile("mov $0x2B, %ax; \
    ltr %ax; \
  ");
}
