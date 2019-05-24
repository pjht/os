#include "cpu/i386/ports.h"
#include <string.h>
#define VGA_BLACK 0
#define VGA_WHITE 15
static char* screen;
static int x=0;

static void set_char(int x,char c) {
  screen[x*2]=c;
  screen[x*2+1]=(VGA_BLACK<<4)|VGA_WHITE;
}

void vga_init(char* addr) {
  screen=addr;
}

void vga_write_string(const char* string) {
  for (size_t i=0;i<strlen(string);i++) {
    if (string[i]=='\n') continue;
    set_char(x,string[i]);
    x++;
  }
}
