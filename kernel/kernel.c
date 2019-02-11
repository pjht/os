#include "../cpu/cpu_init.h"
#include "../cpu/i386/ports.h"
#include "../cpu/i386/tasking.h"
#include "../drivers/vga.h"
#include <grub/text_fb_info.h>
#include <stdlib.h>
#include <tasking.h>
#include "multiboot.h"

void task() {
  vga_write_string("TASK!\n");
  for (;;) {
    yield();
  }
}

void kmain(multiboot_info_t* header) {
  cpu_init();
  text_fb_info info;
  if (header->flags&MULTIBOOT_INFO_FRAMEBUFFER_INFO&&header->framebuffer_type==2) {
    info.address=(char*)((header->framebuffer_addr&0xFFFFFFFF)+0xC0000000);
    info.width=header->framebuffer_width;
    info.height=header->framebuffer_height;
  } else {
    info.address=(char*)0xC00B8000;
    info.width=80;
    info.height=25;
  }
  vga_init(info);
  vga_write_string("Hello\n");
  // asm volatile("  \
  //   cli; \
  //   mov $0x23, %ax; \
  //   mov %ax, %ds; \
  //   mov %ax, %es; \
  //   mov %ax, %fs; \
  //   mov %ax, %gs; \
  //                 \
  //   mov %esp, %eax; \
  //   pushl $0x23; \
  //   pushl %eax; \
  //   pushf; \
  //   pop %eax; \
  //   or $0x200,%eax; \
  //   push %eax; \
  //   pushl $0x1B; \
  //   push $1f; \
  //   iret; \
  // 1: \
  //   ");
  vga_write_string("UMODE!\n");
  port_byte_out(0xe9,'U');
  port_byte_out(0xe9,'M');
  port_byte_out(0xe9,'O');
  port_byte_out(0xe9,'D');
  port_byte_out(0xe9,'E');
  port_byte_out(0xe9,'!');
  port_byte_out(0xe9,'\n');
  vga_write_string("Task create\n");
  tasking_createTask(task);
  vga_write_string("Task switch\n");
  yield();
  vga_write_string("Back in main\n");
}
