#include <grub/text_fb_info.h>
#include "../vga.h"
char* screen;
int width;
int height;
vga_colors fg_color=VGA_WHITE;
vga_colors bg_color=VGA_BLACK;

void vga_init(text_fb_info framebuffer_info) {
  screen=framebuffer_info.address;
  width=framebuffer_info.width;
  height=framebuffer_info.height;
}
