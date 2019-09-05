#include <grub/text_fb_info.h>
#include "vga.h"
#include "ports.h"
#include <string.h>
#include <stddef.h>
#include "cansid.h"
#define xy_to_indx(x,y) ((x+(y*width))*2)
static char* screen;
static int width;
static int height;
static int x;
static int y;
static vga_colors fg_color;
static vga_colors bg_color;
static char* scroll_buf[0xfa0];

static struct cansid_state state;

static void set_char(int x,int y,struct color_char ch) {
  screen[xy_to_indx(x,y)]=ch.ascii;
  screen[xy_to_indx(x,y)+1]=ch.style;
}

void vga_clear() {
  for (int y=0;y<height;y++) {
    for (int x=0;x<width;x++) {
      struct color_char ch;
      ch.ascii=' ';
      ch.style=0;
      set_char(x,y,ch);
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
  state=cansid_init();
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
    } else {
      struct color_char ch=cansid_process(&state,c);
      if (ch.ascii) {
        set_char(x,y,ch);
        x++;
      }
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
      struct color_char ch;
      ch.ascii=' ';
      ch.style=0;
      set_char(x,y,ch);
  }
}
