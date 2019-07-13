#include "vga.h"
#include <grub/text_fb_info.h>
#include <tasking.h>
#include <memory.h>

int main() {
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  for(;;) {
    yield();
  }
}
