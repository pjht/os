#include "../libc/string.h"
#include "vga.h"
#include <grub/text_fb_info.h>

int main() {
  text_fb_info info;
  info.address=map_phys(0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  vga_write_string("INIT VGA\n");
  for (;;);
}
