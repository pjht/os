#include "../isr.h"
#include "idt.h"
#include <cpu/ports.h>
// #include "paging.h"
#include "../halt.h"
#include "../../drivers/vga.h"
// #include "../tasking.h"
#include "interrupt.h"
#include <string.h>
#include <stdint.h>
static isr_t irq_handlers[16];

/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {
    idt_set_gate(0,(uint64_t)isr0);
    idt_set_gate(1,(uint64_t)isr1);
    idt_set_gate(2,(uint64_t)isr2);
    idt_set_gate(3,(uint64_t)isr3);
    idt_set_gate(4,(uint64_t)isr4);
    idt_set_gate(5,(uint64_t)isr5);
    idt_set_gate(6,(uint64_t)isr6);
    idt_set_gate(7,(uint64_t)isr7);
    idt_set_gate(8,(uint64_t)isr8);
    idt_set_gate(9,(uint64_t)isr9);
    idt_set_gate(10,(uint64_t)isr10);
    idt_set_gate(11,(uint64_t)isr11);
    idt_set_gate(12,(uint64_t)isr12);
    idt_set_gate(13,(uint64_t)isr13);
    idt_set_gate(14,(uint64_t)isr14);
    idt_set_gate(15,(uint64_t)isr15);
    idt_set_gate(16,(uint64_t)isr16);
    idt_set_gate(17,(uint64_t)isr17);
    idt_set_gate(18,(uint64_t)isr18);
    idt_set_gate(19,(uint64_t)isr19);
    idt_set_gate(20,(uint64_t)isr20);
    idt_set_gate(21,(uint64_t)isr21);
    idt_set_gate(22,(uint64_t)isr22);
    idt_set_gate(23,(uint64_t)isr23);
    idt_set_gate(24,(uint64_t)isr24);
    idt_set_gate(25,(uint64_t)isr25);
    idt_set_gate(26,(uint64_t)isr26);
    idt_set_gate(27,(uint64_t)isr27);
    idt_set_gate(28,(uint64_t)isr28);
    idt_set_gate(29,(uint64_t)isr29);
    idt_set_gate(30,(uint64_t)isr30);
    idt_set_gate(31,(uint64_t)isr31);
    idt_set_gate(80,(uint64_t)isr80);
    // Remap the PIC
    port_byte_out(0x20,0x11);
    port_byte_out(0xA0,0x11);
    port_byte_out(0x21,0x20);
    port_byte_out(0xA1,0x28);
    port_byte_out(0x21,0x04);
    port_byte_out(0xA1,0x02);
    port_byte_out(0x21,0x01);
    port_byte_out(0xA1,0x01);
    port_byte_out(0x21,0x0);
    port_byte_out(0xA1,0x0);

    // Install the IRQs
    idt_set_gate(32,(uint64_t)irq0);
    idt_set_gate(33,(uint64_t)irq1);
    idt_set_gate(34,(uint64_t)irq2);
    idt_set_gate(35,(uint64_t)irq3);
    idt_set_gate(36,(uint64_t)irq4);
    idt_set_gate(37,(uint64_t)irq5);
    idt_set_gate(38,(uint64_t)irq6);
    idt_set_gate(39,(uint64_t)irq7);
    idt_set_gate(40,(uint64_t)irq8);
    idt_set_gate(41,(uint64_t)irq9);
    idt_set_gate(42,(uint64_t)irq10);
    idt_set_gate(43,(uint64_t)irq11);
    idt_set_gate(44,(uint64_t)irq12);
    idt_set_gate(45,(uint64_t)irq13);
    idt_set_gate(46,(uint64_t)irq14);
    idt_set_gate(47,(uint64_t)irq15);

    load_idt();
    
    asm volatile("sti");
}


/* To print the message which defines every exception */
static char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t* r) {
  switch (r->int_no) {
    case 14: {
      uint64_t addr;
      asm("movq %%cr2,%0": "=r"(addr));
      if (r->err_code==0) {
        vga_write_string("Kernel process tried to read a non-present page entry at address ");
      } else if (r->err_code==1) {
        vga_write_string("Kernel process tried to read a page and caused a protection fault at address ");
      } else if (r->err_code==2) {
        vga_write_string("Kernel process tried to write to a non-present page entry at address ");
      } else if (r->err_code==3) {
        vga_write_string("Kernel process tried to write a page and caused a protection fault at address ");
      } else if (r->err_code==4) {
        vga_write_string("User process tried to read a non-present page entry at address ");
      } else if (r->err_code==5) {
        vga_write_string("User process tried to read a page and caused a protection fault at address ");
      } else if (r->err_code==6) {
        vga_write_string("User process tried to write to a non-present page entry at address ");
      } else if (r->err_code==7) {
        vga_write_string("User process tried to write a page and caused a protection fault at address ");
      }
      char str[11];
      str[0]='\0';
      hex_to_ascii(addr,str);
      vga_write_string(str);
      vga_write_string("\n");
      // if ((r->err_code&1)==0) {
      //   // int dir_entry=(addr&0xFFC00000)>>22;
      //   // int table_entry=(addr&0x3FF000)>12;
      //   // if (dir_entry_present(dir_entry)) {
      //   //   set_table_entry(dir_entry,table_entry,((dir_entry*1024)+table_entry)*0x1000,1,1,1);
      //   //   for(int page=0;page<1024;page++) {
      //   //     asm volatile("invlpg (%0)"::"r"(((dir_entry*1024)+page)*0x1000):"memory");
      //   //   }
      //   // } else {
      //   //   for(int page=0;page<1024;page++) {
      //   //     set_table_entry(dir_entry,page,0x0,1,1,0);
      //   //   }
      //   //   set_table_entry(dir_entry,table_entry,((dir_entry*1024)+table_entry)*0x1000,1,1,1);
      //   //   set_directory_entry(dir_entry,dir_entry,1,1,1);
      //   // }
      //   // return;
      // }
      halt();
      break;
    case 80:
      // if (r->eax==1) {
      //   tss_stack_reset();
      //   tasking_yield();
      // } else if (r->eax==2) {
      //   tasking_createTask((void*)r->ebx);
      // } else if (r->eax==3) {
      //   r->ebx=(uint64_t)alloc_pages(r->ebx);
      // } else if (r->eax==4) {
      //   alloc_pages_virt(r->ebx,(void*)r->ecx);
      // } else if (r->eax==5) {
      //   r->ebx=(uint64_t)tasking_get_errno_address();
      // }
    break;
    }
  }
}

void isr_register_handler(uint8_t n,isr_t handler) {
    if (n>16) {
      return;
    }
    irq_handlers[n] = handler;
}

void irq_handler(registers_t* r) {
    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r->int_no >= 40) port_byte_out(0xA0,0x20); /* slave */
    port_byte_out(0x20,0x20); /* master */
    /* Handle the interrupt in a more modular way */
    if (irq_handlers[r->int_no-32] != NULL) {
        isr_t handler = irq_handlers[r->int_no];
        handler(r);
    }
}
