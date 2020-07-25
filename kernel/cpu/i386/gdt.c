/**
 * \file 
*/

#include <stdint.h>
#include <string.h>

#define NUM_ENTRIES 6 //!< Number of entries in the GDT

extern uint32_t int_stack_top; //!< Initial kernel stack before the kernel's first yield
/**
 * Represents an entry in the GDT.
*/
typedef struct {
  uint16_t limit_low16;//!< Low 16 bits of the limit
  uint16_t base_low16; //!< Low 16 bits of the base
  uint8_t base_mid8; //!< Middle 8 bits of the base
  uint8_t access; /**<
                      Access byte. Gives info about the descriptor. <br>
                      Format: <br>
                      Bit 7: Present. Must be 1 for all valid selectors. <br>
                      Bits 6-5. Privilege. Contains the ring level for the selector. 0 for kernel mode, 3 for user mode. <br>
                      Bit 4. Descriptor type. Must be set for code/data segments and cleared for system segments like the TSS. <br>
                      Bit 3. Executable. If this bit is set, it is a code selector, otherwise a data selector. <br>
                      Bit 2. Direction/Conforming. Too complex to explain, should be set to 0. <br>
                      Bit 1. Readable/Writable. For code sels, this bit sets whther you can use it like a read-only data segment. For data sels, it sets whether the selector is writable. <br>
                      Bit 0. Acessed bit. Set to 0. <br>
                  */
  uint8_t limit_flags; /**< 
                            High nibble of this contains two flags, and the lower niblle contains the high 4 bits of the limit. <br>
                            The flags are: <br>
                            Bit 3. Granularity. 0 for byte granularity, 1 for 4 KB granularity. <br>
                            Bit 2. Size. 0 for 16 bit protected mode, 1 for 32 bit protected mode. <br>
                            Bits 1-0. Unused. Set to 0. <br>
                       */
  uint8_t base_high8; //!< High 8 bits of the base
} __attribute__((packed)) gdt_entry;

/**
 * Pointed to by the GDTR to tell the processor the GDT's size and address.
*/
typedef struct {
  uint16_t size; //!< Size of the GDT.
  gdt_entry* address; //!< Address of the GDT.
} __attribute__((packed)) gdt_description;

/**
 * Represents a TSS.
*/
typedef struct {
   uint32_t prev_tss;   //!< The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       //!< The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        //!< The stack segment to load when we change to kernel mode.
   uint32_t esp1;       //!< Unused
   uint32_t ss1;        //!< Unused
   uint32_t ss2;        //!< Unused
   uint32_t esp2;       //!< Unused
   uint32_t cr3;        //!< Unused
   uint32_t eip;        //!< Unused
   uint32_t eflags;     //!< Unused
   uint32_t eax;        //!< Unused
   uint32_t ecx;        //!< Unused
   uint32_t edx;        //!< Unused
   uint32_t ebx;        //!< Unused
   uint32_t esp;        //!< Unused
   uint32_t ebp;        //!< Unused
   uint32_t esi;        //!< Unused
   uint32_t edi;        //!< Unused
   uint32_t es;         //!< The value to load into ES when we change to kernel mode.
   uint32_t cs;         //!< The value to load into CS when we change to kernel mode.
   uint32_t ss;         //!< The value to load into SS when we change to kernel mode.
   uint32_t ds;         //!< The value to load into DS when we change to kernel mode.
   uint32_t fs;         //!< The value to load into FS when we change to kernel mode.
   uint32_t gs;         //!< The value to load into GS when we change to kernel mode.
   uint32_t ldt;        //!< Unused
   uint16_t trap;       //!< Unused
   uint16_t iomap_base; //!< Offset of the IOPB in the TSS.
   char iopb[8192];     //!< IO port bitmap
   uint8_t set_ff;      //!< Must be set to 0xFF to mark the end of the IOPB
} __attribute__((packed)) tss_entry;

static gdt_entry gdt[NUM_ENTRIES]; //!< The GDT
static gdt_description gdt_desc; //!< The value to load into the GDTR
tss_entry tss; //!< The TSS

/**
 * Set a GDT entry.
 * \param i The GDT entry to set.
 * \param base The base of the GDT entry.
 * \param limit The limit of the GDT entry.
 * \param access The access byte of the GDT entry.
*/

static void set_entry(int i,uint32_t base,uint32_t limit,uint8_t access) {
  gdt[i].limit_low16=limit&0xFFFF;
  gdt[i].base_low16=base&0xFFFFF;
  gdt[i].base_mid8=(base&0xFF0000)>>16;
  gdt[i].access=access;
  uint8_t limit_high4=(limit&0xF0000)>>16;
  gdt[i].limit_flags=0xC0|limit_high4;
  gdt[i].base_high8=(base&0xFF000000)>>24;
}

/**
 * Set a GDT entry.
 * \param num The GDT entry to set.
 * \param ss0 The kernel stack selector.
 * \param esp0 The kernel stack pointer.
*/

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
  set_entry(1,0,0xFFFFF,0b10011010);
  set_entry(2,0,0xFFFFF,0b10010010);
  set_entry(3,0,0xFFFFF,0b11111010);
  set_entry(4,0,0xFFFFF,0b11110010);
  write_tss(5,0x10,int_stack_top+0xC0000000);
  gdt_desc.size=(sizeof(gdt_entry)*NUM_ENTRIES)-1;
  gdt_desc.address=gdt;
  asm volatile(" \
                lgdt (%%eax); \
                jmp $0x8,$gdt_init_asm_code_upd; \
                gdt_init_asm_code_upd: \
                mov $0x10, %%ax; \
                mov %%ax, %%ds; \
                mov %%ax, %%ss; \
                mov %%ax, %%es; \
                mov %%ax, %%fs; \
                mov %%ax, %%gs; \
              "::"a"((uint32_t)&gdt_desc));
  asm volatile("mov $0x2B, %ax; \
    ltr %ax; \
  ");
}
