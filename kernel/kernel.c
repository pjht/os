#include "../drivers/vga.h"
#include "../drivers/isr.h"
#include "../drivers/idt.h"
#include "../drivers/gdt.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "syscalls.h"

char* line=0;

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
  write_string("Keyboard initialized\n");
  switch_to_user_mode();
  syscall_write_string("MYOS V 1.0\n");
  syscall_write_string(">");
  char buf[256];
  do {
    syscall_gets(buf);
  } while (buf[0]=='\0');
  syscall_write_string(buf);
  syscall_write_string("HALTING");
}

void user_input(char* str) {
  line=str;
}

void kgets(char* buf) {
  if (line==0) {
    buf[0]='\0';
    return;
  }
  strcpy(buf,line);
  line=0;
}
