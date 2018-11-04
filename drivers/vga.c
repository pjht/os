#include "ports.h"
#include "vga.h"
#include "../libc/memory.h"

#define SCREEN_CTRL 0x3d4
#define SCREEN_DATA 0x3d5
#define VIDEO_MEMORY 0xc00b8000
#define VIDEO_MEMORY_PG1 0xc00b8fa0
static char* video_memory;
static char x;
static char y;
static char color;


void set_cursor_offset(int offset);

int get_offset(int x,int y) {
  return 2*(y*VGA_WIDTH+x);
}

void init_vga(VGA_COLOR txt,VGA_COLOR bg) {
  video_memory=(char*)VIDEO_MEMORY;
  x=0;
  y=0;
  color=(int)bg;
  color=color<<4;
  color=color|txt;
  clear_screen();
}

void write_string(const char *string) {
    char c;
    while(*string!=0) {
      if (y==25) {
        memory_copy((char*)(VIDEO_MEMORY+get_offset(0,1)),(char*)VIDEO_MEMORY_PG1,get_offset(0,24));
        clear_screen();
        memory_copy((char*)VIDEO_MEMORY_PG1,(char*)VIDEO_MEMORY,get_offset(0,25));
        x=0;
        y=24;
      }
      c=*string;
      string++;
      if (c=='\n') {
        x=0;
        y++;
        set_cursor_offset(get_offset(x,y));
        continue;
      }
      video_memory[get_offset(x,y)]=c;
      video_memory[get_offset(x,y)+1]=color;
      x++;
      if (x==80) {
        x=0;
        y++;
      }
      set_cursor_offset(get_offset(x,y));
    }
}

void screen_backspace() {
  if (x>0) {
    x--;
    video_memory[get_offset(x,y)]=' ';
    video_memory[get_offset(x,y)+1]=color;
    set_cursor_offset(get_offset(x,y));
  }
}

void clear_screen() {
  int x=0;
  int y=0;
  while (y<25) {
    video_memory[get_offset(x,y)]=' ';
    video_memory[get_offset(x,y)+1]=color;
    x++;
    if(x==80) {
      x=0;
      y++;
    }
  }
  x=0;
  y=0;
  set_cursor_offset(0);
}

void set_cursor_offset(int offset) {
  offset/=2;
  port_byte_out(SCREEN_CTRL,14);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset>>8));
  port_byte_out(SCREEN_CTRL,15);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset&0xff));
}
