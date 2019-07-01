#include <grub/text_fb_info.h>
#include "vga.h"
#include "ports.h"
#include <string.h>
#include <stddef.h>
#define xy_to_indx(x,y) ((x+(y*width))*2)
static char* screen;
static int width;
static int height;
static int x;
static int y;
static vga_colors fg_color;
static vga_colors bg_color;
static char* scroll_buf[0xfa0];


static void set_char(int x,int y,char c) {
  screen[xy_to_indx(x,y)]=c;
  screen[xy_to_indx(x,y)+1]=(bg_color<<4)|fg_color;
}

void vga_clear() {
  for (int y=0;y<height;y++) {
    for (int x=0;x<width;x++) {
      set_char(x,y,' ');
    }
  }
}

static void set_cursor(int x,int y) {
  int pos=(x+(y*width));
  port_byte_out(0x3D4,0xF);
  port_byte_out(0x3D5,pos&0xFF);
  port_byte_out(0x3D4,0xE);
  port_byte_out(0x3D5,(pos&0xFF00)>>8);
}

void vga_init(text_fb_info framebuffer_info) {
  x=0;
  y=0;
  fg_color=VGA_WHITE;
  bg_color=VGA_BLACK;
  screen=framebuffer_info.address;
  width=framebuffer_info.width;
  height=framebuffer_info.height;
  port_byte_out(0x3D4,0xA);
  port_byte_out(0x3D5,(port_byte_in(0x3D5)&0xC0)|14);
  port_byte_out(0x3D4,0xB);
  port_byte_out(0x3D5,(port_byte_in(0x3D5)&0xE0)|15);
  set_cursor(0,0);
  vga_clear();
}

void vga_write_string(const char* string) {
  for (size_t i=0;i<strlen(string);i++) {
    char c=string[i];
    if (c=='\n') {
      x=0;
      y++;
      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wunused-value"
      for (int i=0;i<67108864;i++) {
        1+1;
      }
      #pragma GCC diagnostic pop
    } else {
      set_char(x,y,c);
      x++;
    }
    if (x==width) {
      x=0;
      y++;
    }
    if (y==height) {
      x=0;
      y=24;
      memcpy(scroll_buf,&screen[xy_to_indx(0,1)],xy_to_indx(0,24));
      vga_clear();
      memcpy(screen,scroll_buf,xy_to_indx(0,25));
    }
  }
  set_cursor(x,y);
}

void vga_backspace() {
  if (x!=0) {
      x--;
      set_char(x,y,' ');
      set_cursor(x,y);
  }
}
