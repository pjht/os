#include "../cpu/cpu_init.h"
#include "../cpu/i386/paging.h"
#include "../drivers/vga.h"
#include <grub/text_fb_info.h>
#include <stdlib.h>
#include <tasking.h>
#include <k_messages.h>
#include <string.h>
#include "multiboot.h"

void init() {
  void* addr=get_paddr((void*)0xC0000001);
  vga_write_string("Physical address of 0xC0000001 is ");
  char str[11];
  str[0]='\0';
  hex_to_ascii((uint32_t)addr,str);
  vga_write_string(str);
  vga_write_string("\n");
  for(;;) {
    yield();
  }
}

void kmain(multiboot_info_t* header) {
  cpu_init();
  text_fb_info info;
  if (header->flags&MULTIBOOT_INFO_FRAMEBUFFER_INFO&&header->framebuffer_type==2) {
    info.address=(char*)(((uint32_t)header->framebuffer_addr&0xFFFFFFFF)+0xC0000000);
    info.width=header->framebuffer_width;
    info.height=header->framebuffer_height;
  } else {
    info.address=(char*)0xC00B8000;
    info.width=80;
    info.height=25;
  }
  vga_init(info);
  createTask(init);
  for (;;) {
    uint32_t sender;
    char* msg;
    msg=get_msg(&sender);
    if (msg) {
      char* cmd=strtok(msg," ");
      if (strcmp(cmd,"GET_PADDR")==0) {
        uint32_t addr;
        addr=addr|msg[10];
        addr=addr|msg[11]<<8;
        addr=addr|msg[12]<<16;
        addr=addr|msg[13]<<24;
        addr=(uint32_t)virt_to_phys((void*)addr);
        char* addr_str=malloc(sizeof(char)*5);
        addr_str[4]=0;
        addr_str[0]=addr&0xFF;
        addr_str[1]=(addr&0xFF00)>>8;
        addr_str[2]=(addr&0xFF0000)>>16;
        addr_str[3]=(addr&0xFF0000)>>24;
        send_msg(sender,addr_str);
      }
    }
    yield();
  }
}
