#include "vga.h"
#include <cpu/ports.h>
#include <string.h>
#include <stddef.h>
#include <memory.h>
#include "vtconsole.h"
#define xy_to_indx(x,y) ((x+(y*width))*2)
static vga_color colors[8];
static char* screen;
static int width;
static int height;

static vtconsole_t* console;

static void set_char(int x,int y,char ch,char style) {
  screen[xy_to_indx(x,y)]=ch;
  screen[xy_to_indx(x,y)+1]=style;
}

static void vt_set_char(struct vtconsole* vtc, vtcell_t* cell, int x, int y) {
  vga_color fg_color=colors[cell->attr.fg];
  vga_color bg_color=colors[cell->attr.bg];
  char style=(fg_color&0xF)|((bg_color&0xF)<<4);
  style=style|((cell->attr.bright&0x1)<<7);
  set_char(x,y,cell->c,style);
}

void vga_clear() {
  for (int y=0;y<height;y++) {
    for (int x=0;x<width;x++) {
      set_char(x,y,' ',0);
    }
  }
}

static void set_cursor(struct vtconsole* vtc, vtcursor_t* cur) {
  // serial_print("CURSOR AT X:");
  // char str[128];
  // int_to_ascii(cur->x,str);
  // serial_print(str);
  // serial_print(" Y:");
  // int_to_ascii(cur->y,str);
  // serial_print(str);
  // serial_print("\n");
  // int pos=(cur->x+(cur->y*width));
  // port_byte_out(0x3D4,0xF);
  // port_byte_out(0x3D5,pos&0xFF);
  // port_byte_out(0x3D4,0xE);
  // port_byte_out(0x3D5,(pos&0xFF00)>>8);
}

void vga_init() {
  colors[0]=VGA_BLACK;
  colors[1]=VGA_RED;
  colors[2]=VGA_GREEN;
  colors[3]=VGA_YELLOW;
  colors[4]=VGA_BLUE;
  colors[5]=VGA_PURPLE;
  colors[6]=VGA_CYAN;
  colors[7]=VGA_WHITE;
  screen=map_phys((void*)0xB8000,10);
  width=80;
  height=25;
  console=vtconsole(width,height,vt_set_char,set_cursor);
  // port_byte_out(0x3D4,0xA);
  // port_byte_out(0x3D5,(port_byte_in(0x3D5)&0xC0)|14);
  // port_byte_out(0x3D4,0xB);
  // port_byte_out(0x3D5,(port_byte_in(0x3D5)&0xE0)|15);
  vtcursor_t cur;
  cur.x=0;
  cur.y=0;
  set_cursor(NULL,&cur);
  vga_clear();
}

void vga_write_string(const char* string) {
  vtconsole_write(console,string,strlen(string));
}
