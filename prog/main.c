#include "../libc/string.h"
#include <grub/text_fb_info.h>

int main() {
  text_fb_info info;
  info.address=(char*)0xC00B8000;
  info.width=80;
  info.height=25;
  vga_init(info);
  vga_write_string("INIT VGA\n");
  for (;;);
}
