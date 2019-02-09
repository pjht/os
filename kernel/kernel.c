#include "../cpu/cpu_init.h"
#include <grub/text_fb_info.h>
#include <stdlib.h>
#include "multiboot.h"

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
}
