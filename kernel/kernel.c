#include "../drivers/vga.h"
#include "../drivers/isr.h"
#include "../drivers/idt.h"
#include "../drivers/gdt.h"
#include "../drivers/paging.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "../drivers/serial.h"
#include "syscalls.h"

char line[256];
char input=0;

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
  serial_full_configure(SERIAL_COM1_BASE,12);
  write_string("Initialized COM1 at 9600 baud\n");
  isr_install();
  asm volatile("sti");
  write_string("Setup interrupts\n");
  init_gdt();
  write_string("Setup new GDT\n");
  init_paging();
  write_string("Setup paging\n");
  init_keyboard();
  write_string("Keyboard initialized\n");
  switch_to_user_mode();
  syscall_write_string("MYOS V 1.0\n");
  int mb=8;
  while (1) {
    syscall_write_string(">");
    char buf[256];
    do {
      syscall_gets(buf);
    } while (buf[0]=='\0');
    if (strcmp("pf\n",buf)==0) {
        int* ptr=(int*)(mb*1048576);
        char str[20];
        int_to_ascii(mb,str);
        write_string("Storing ");
        write_string(str);
        str[0]='\0';
        hex_to_ascii(ptr,str);
        write_string(" into ");
        write_string(str);
        write_string("\n");
        *ptr=mb;
        write_string("Loading value\n");
        int x=*ptr;
        write_string("Got ");
        str[0]='\0';
        int_to_ascii(x,str);
        write_string(str);
        write_string("\n");
        mb+=1;
    }
  }
}

void user_input(char* str) {
  strcpy(line,str);
  input=1;
}

void kgets(char* buf) {
  if (input) {
    strcpy(buf,line);
    input=0;
  } else {
    buf[0]='\0';
    return;
  }
}
