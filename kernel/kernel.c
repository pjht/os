#include "../drivers/vga.h"
#include "../drivers/isr.h"
#include "../drivers/idt.h"
#include "../drivers/gdt.h"
#include "syscalls.h"
#include "keyboard.h"
void switch_to_user_mode() {
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
    pop %eax; \
    or $0x200,%eax; \
    push %eax; \
    pushl $0x1B; \
    push $1f; \
    iret; \
  1: \
    ");
}


void main() {
  init_vga(WHITE,BLACK);
  write_string("Initialized VGA\n");
  isr_install();
  asm volatile("sti");
  write_string("Setup interrupts\n");
  init_gdt();
  write_string("Setup new GDT\n");
  init_keyboard();
  switch_to_user_mode();
  syscall_write_string("User mode!\n");
}

void user_input(char* str) {
  syscall_write_string(str);
  return;
}
