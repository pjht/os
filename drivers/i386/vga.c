#include <grub/text_fb_info.h>
#include "../vga.h"
#define xy_to_mem(x,y) ((x+(y*height))*2)
char* screen;
int width;
int height;
vga_colors fg_color;
vga_colors bg_color;

void vga_set_char(int x,int y,char c) {
  screen[xy_to_mem(x,y)]=c;
  screen[xy_to_mem(x,y)+1]=(bg_color<<4)|fg_color;
}

void vga_init(text_fb_info framebuffer_info) {
  fg_color=VGA_WHITE;
  bg_color=VGA_BLACK;
  screen=framebuffer_info.address;
  width=framebuffer_info.width;
  height=framebuffer_info.height;
  vga_clear();
}

void vga_clear() {
  for (int y=0;y<height;y++) {
    for (int x=0;x<width;x++) {
      vga_set_char(x,y,' ');
    }
  }
}
