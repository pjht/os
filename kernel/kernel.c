#include "../drivers/vga.h"
#include "../drivers/isr.h"
#include "../drivers/idt.h"
/*
pop %eax; \
or $0x200,%eax; \
push %eax; \
*/
void switch_to_user_mode() {
   // Set up a stack structure for switching to user mode.
   asm volatile("  \
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pushl $0x1B; \
     push $1f; \
     iret; \
     1: \
     ");
}


void main() {
  init_vga(GRAY,BLACK);
  write_string("Initialized VGA\n");
  isr_install();
  asm volatile("sti");
  write_string("Setup interrupts\n");
  switch_to_user_mode();
}
