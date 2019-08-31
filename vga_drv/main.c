#include <grub/text_fb_info.h>

int main() {
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
}
