#include "../drivers/vga.h"
#include "../drivers/isr.h"
#include "../drivers/idt.h"
#include "../drivers/gdt.h"
/*
pop %eax; \
or $0x200,%eax; \
push %eax; \
*/
void switch_to_user_mode() {
  // set_kernel_stack(0x80000);
  // Set up a stack structure for switching to user mode.
  asm volatile("  \
    cli; \
    mov $0x23, %ax; \
    mov %ax, %ds; \
    mov %ax, %es; \
    mov %ax, %fs; \
    mov %ax, %gs; \
                  \
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
  init_gdt();
  write_string("Setup new GDT\n");
  switch_to_user_mode();
  write_string("User mode!\n");
}
