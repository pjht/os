#include "ports.h"
#include "vga.h"
#include "../libc/memory.h"
#define SCREEN_CTRL 0x3d4
#define SCREEN_DATA 0x3d5
#define VIDEO_MEMORY 0xb8000
#define VIDEO_MEMORY_PG1 0xb8fa0
static char* video_memory;
static char x;
static char y;
static char color;


int get_cursor_offset();
void set_cursor_offset(int offset);

int get_offset(int x,int y) {
  return 2*(y*VGA_WIDTH+x);
}
int get_offset_y(int offset) {
  return offset/(2*VGA_WIDTH);
}

int get_offset_x(int offset) {
  return (offset-(get_offset_y(offset)*2*VGA_WIDTH))/2;
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
        memory_copy(VIDEO_MEMORY+get_offset(0,1),VIDEO_MEMORY_PG1,get_offset(0,24));
        clear_screen();
        memory_copy(VIDEO_MEMORY_PG1,VIDEO_MEMORY,get_offset(0,25));
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


int get_cursor_offset() {
  port_byte_out(SCREEN_CTRL,14);
  int offset=port_byte_in(SCREEN_DATA)<<8;
  port_byte_out(SCREEN_CTRL,15);
  offset+=port_byte_in(SCREEN_DATA);
  return offset*2;
}

void set_cursor_offset(int offset) {
  offset/=2;
  port_byte_out(SCREEN_CTRL,14);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset>>8));
  port_byte_out(SCREEN_CTRL,15);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset&0xff));
}
