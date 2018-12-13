#include "../../cpu/i386/ports.h"
#include "../screen.h"
#include "../../libc/string.h"

#define SCREEN_CTRL 0x3d4
#define SCREEN_DATA 0x3d5
#define VIDEO_MEMORY 0xc00b8000
#define VIDEO_MEMORY_PG1 0xc00b8fa0
#define COLOR 0xF
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static char* video_memory;
static char x;
static char y;

void screen_set_cursor_offset(int offset);

int screen_get_offset(int x,int y) {
  return 2*(y*VGA_WIDTH+x);
}

void screen_init() {
  video_memory=(char*)VIDEO_MEMORY;
  x=0;
  y=0;
  screen_clear();
}

void screen_write_string(const char *string) {
    char c;
    while(*string!=0) {
      if (y==25) {
        memcpy((void*)VIDEO_MEMORY_PG1,(void*)(VIDEO_MEMORY+screen_get_offset(0,1)),screen_get_offset(0,24));
        screen_clear();
        memcpy((void*)VIDEO_MEMORY,(void*)VIDEO_MEMORY_PG1,screen_get_offset(0,25));
        x=0;
        y=24;
      }
      c=*string;
      string++;
      if (c=='\n') {
        x=0;
        y++;
        screen_set_cursor_offset(screen_get_offset(x,y));
        continue;
      }
      video_memory[screen_get_offset(x,y)]=c;
      video_memory[screen_get_offset(x,y)+1]=COLOR;
      x++;
      if (x==80) {
        x=0;
        y++;
      }
      screen_set_cursor_offset(screen_get_offset(x,y));
    }
}

void screen_backspace() {
  if (x>0) {
    x--;
    video_memory[screen_get_offset(x,y)]=' ';
    video_memory[screen_get_offset(x,y)+1]=COLOR;
    screen_set_cursor_offset(screen_get_offset(x,y));
  }
}

void screen_clear() {
  int x=0;
  int y=0;
  while (y<25) {
    video_memory[screen_get_offset(x,y)]=' ';
    video_memory[screen_get_offset(x,y)+1]=COLOR;
    x++;
    if(x==80) {
      x=0;
      y++;
    }
  }
  x=0;
  y=0;
  screen_set_cursor_offset(0);
}

void screen_set_cursor_offset(int offset) {
  offset/=2;
  port_byte_out(SCREEN_CTRL,14);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset>>8));
  port_byte_out(SCREEN_CTRL,15);
  port_byte_out(SCREEN_DATA,(unsigned char)(offset&0xff));
}
