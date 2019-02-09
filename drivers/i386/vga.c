#include <grub/text_fb_info.h>
#include "../vga.h"
#include <string.h>
#define xy_to_indx(x,y) ((x+(y*height))*2)
char* screen;
int width;
int height;
int x;
int y;
vga_colors fg_color;
vga_colors bg_color;

void vga_set_char(int x,int y,char c) {
  screen[xy_to_indx(x,y)]=c;
  screen[xy_to_indx(x,y)+1]=(bg_color<<4)|fg_color;
}

void vga_init(text_fb_info framebuffer_info) {
  x=0;
  y=0;
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

void vga_write_string(const char* string) {
  for (int i=0;i<strlen(string);i++) {
    char c=string[i];
    if (c=='\n') {
      x=0;
      y++;
    } else {
      vga_set_char(x,y,c);
      x++;
    }
    if (x==width) {
      x=0;
      y++;
    }
    if (y==height) {
      x=0;
      y=0;
      char* pg1=(char*)((uint32_t)screen+0xfa0);
      memcpy(pg1,&screen[xy_to_indx(0,1)],xy_to_indx(0,24));
      vga_clear();
      memcpy(&screen,pg1,xy_to_indx(0,25));
    }
  }
}
