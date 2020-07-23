#include "../../kernel.h"
#include "../../tasking.h"
#include "../../vga_err.h"
#include "../../address_spaces.h"
#include "../halt.h"
#include "../isr.h"
#include "../paging.h"
#include "../serial.h"
#include "gdt.h"
#include "idt.h"
#include "interrupt.h"
#include <cpu/ports.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscalls.h>
#include <sys/types.h>

void irq_handler(registers_t* r);
static isr_t irq_handlers[16];

/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {
    idt_set_gate(0,(uint32_t)isr0);
    idt_set_gate(1,(uint32_t)isr1);
    idt_set_gate(2,(uint32_t)isr2);
    idt_set_gate(3,(uint32_t)isr3);
    idt_set_gate(4,(uint32_t)isr4);
    idt_set_gate(5,(uint32_t)isr5);
    idt_set_gate(6,(uint32_t)isr6);
    idt_set_gate(7,(uint32_t)isr7);
    idt_set_gate(8,(uint32_t)isr8);
    idt_set_gate(9,(uint32_t)isr9);
    idt_set_gate(10,(uint32_t)isr10);
    idt_set_gate(11,(uint32_t)isr11);
    idt_set_gate(12,(uint32_t)isr12);
    idt_set_gate(13,(uint32_t)isr13);
    idt_set_gate(14,(uint32_t)isr14);
    idt_set_gate(15,(uint32_t)isr15);
    idt_set_gate(16,(uint32_t)isr16);
    idt_set_gate(17,(uint32_t)isr17);
    idt_set_gate(18,(uint32_t)isr18);
    idt_set_gate(19,(uint32_t)isr19);
    idt_set_gate(20,(uint32_t)isr20);
    idt_set_gate(21,(uint32_t)isr21);
    idt_set_gate(22,(uint32_t)isr22);
    idt_set_gate(23,(uint32_t)isr23);
    idt_set_gate(24,(uint32_t)isr24);
    idt_set_gate(25,(uint32_t)isr25);
    idt_set_gate(26,(uint32_t)isr26);
    idt_set_gate(27,(uint32_t)isr27);
    idt_set_gate(28,(uint32_t)isr28);
    idt_set_gate(29,(uint32_t)isr29);
    idt_set_gate(30,(uint32_t)isr30);
    idt_set_gate(31,(uint32_t)isr31);
    idt_set_gate(80,(uint32_t)isr80);
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
    idt_set_gate(32,(uint32_t)irq0);
    idt_set_gate(33,(uint32_t)irq1);
    idt_set_gate(34,(uint32_t)irq2);
    idt_set_gate(35,(uint32_t)irq3);
    idt_set_gate(36,(uint32_t)irq4);
    idt_set_gate(37,(uint32_t)irq5);
    idt_set_gate(38,(uint32_t)irq6);
    idt_set_gate(39,(uint32_t)irq7);
    idt_set_gate(40,(uint32_t)irq8);
    idt_set_gate(41,(uint32_t)irq9);
    idt_set_gate(42,(uint32_t)irq10);
    idt_set_gate(43,(uint32_t)irq11);
    idt_set_gate(44,(uint32_t)irq12);
    idt_set_gate(45,(uint32_t)irq13);
    idt_set_gate(46,(uint32_t)irq14);
    idt_set_gate(47,(uint32_t)irq15);

    load_idt();
    
    asm volatile("sti");
}


/* To print the message which defines every exception */

__attribute__((unused)) static char *exception_messages[] = {
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
  if (r->int_no!=80 && r->int_no!=14) {
    serial_write_string(exception_messages[r->int_no]);
  }
  switch (r->int_no) {
    case 14: {
      serial_write_string("PAGE FAULT\n");
      uint32_t addr;
      asm("movl %%cr2,%0": "=r"(addr));
      // serial_write_string("In PID ");
      char str[11];
      // int_to_ascii(getPID(),str);
      // serial_write_string(str);
      serial_write_string(" and address ");
      str[0]='\0';
      hex_to_ascii(r->eip,str);
      serial_write_string(str);
      if (r->err_code==0) {
        serial_write_string(", kernel process tried to read a non-present page entry at address ");
      } else if (r->err_code==1) {
        serial_write_string(", kernel process tried to read a page and caused a protection fault at address ");
      } else if (r->err_code==2) {
        serial_write_string(", kernel process tried to write to a non-present page entry at address ");
      } else if (r->err_code==3) {
        serial_write_string(", kernel process tried to write a page and caused a protection fault at address ");
      } else if (r->err_code==4) {
        serial_write_string(", user process tried to read a non-present page entry at address ");
      } else if (r->err_code==5) {
        serial_write_string(", user process tried to read a page and caused a protection fault at address ");
      } else if (r->err_code==6) {
        serial_write_string(", user process tried to write to a non-present page entry at address ");
      } else if (r->err_code==7) {
        serial_write_string(", user process tried to write a page and caused a protection fault at address ");
      }
      str[0]='\0';
      hex_to_ascii(addr,str);
      serial_write_string(str);
      serial_write_string(".");
      serial_write_string(" Stack is at ");
      str[0]='\0';
      hex_to_ascii(r->useresp,str);
      serial_write_string(str);
      serial_write_string(".\n");
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
      switch (r->eax) {
      case SYSCALL_CREATEPROC:
        tasking_createTask((void*)r->ebx,(void*)r->ecx,0,r->edx,r->esi,r->edx,r->edi,0);
        break;
      case SYSCALL_YIELD:
        tasking_yield();
        break;
      case SYSCALL_BLOCK:
        tasking_block(r->ebx);
        break;
      case SYSCALL_UNBLOCK:
        tasking_unblock(r->ebx,r->ecx);
        break;
      case SYSCALL_EXIT:
        tasking_exit((uint8_t)r->ebx);
        break;
      case SYSCALL_GET_ERRNO_ADDR:
        r->ebx=(uint32_t)tasking_get_errno_address();
        break;
      case SYSCALL_GET_PID:
        r->ebx=(pid_t)tasking_getPID();
        break;
      case SYSCALL_ALLOC_MEM:
        if ((void*)r->ecx==NULL) {
          r->ebx=(uint32_t)alloc_pages(r->ebx);
        } else {
          alloc_pages_virt(r->ebx,(void*)r->ecx);
        }
        break;
      case SYSCALL_PRIV_MAP_PAGES:
        if (tasking_isPrivleged()) {
          r->ebx=0;
          return;
        }
        void* virt_addr=find_free_pages(r->ecx);
        map_pages(virt_addr,(void*)r->ebx,r->ecx,1,1);
        r->ebx=(uint32_t)virt_addr;
        break;
      case SYSCALL_NEW_ADDR_SPACE:
        r->ebx=(uint32_t)paging_new_address_space();
        break;
      case SYSCALL_ADDR_SPACES_COPY_DATA:
        serial_printf("address_spaces_copy_data(0x%x,0x%x,0x%x,0x%x);\n",(void*)r->ebx,(void*)r->ecx,r->edx,(void*)r->esi);
        if ((void*)r->esi!=NULL) {
          address_spaces_copy_data((void*)r->ebx,(void*)r->ecx,r->edx,(void*)r->esi);
        } else {
          r->ebx=(uint32_t)address_spaces_put_data((void*)r->ebx,(void*)r->ecx,r->edx);
        }
        break;
      case SYSCALL_SERIAL_PRINT:
        serial_write_string((char*)r->ebx);
        break;
      case SYSCALL_GET_INITRD_SZ:
        serial_printf("Initrd size is %d bytes\n",initrd_sz);
        r->ebx=initrd_sz;
        break;
      case SYSCALL_COPY_INITRD:
        serial_printf("Copying initrd\n");
        memcpy((char*)r->ebx,initrd,initrd_sz);
        break;
      case SYSCALL_NEW_THREAD: {
        uint32_t tid=tasking_new_thread((void*)r->ebx,tasking_getPID(),1,r->edx);
        if ((uint32_t*)r->ecx!=NULL) {
          *((uint32_t*)r->ecx)=tid;
        }
      }
      break;
      default:
        break;
      } 
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
