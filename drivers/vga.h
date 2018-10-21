#ifndef VGA_H
#define VGA_H

typedef enum {
  BLACK=0,
  BLUE=1,
  GREEN=2,
  CYAN=3,
  RED=4,
  PURPLE=5,
  BROWN=6,
  GRAY=7,
  DARK_GRAY=8,
  LIGHT_BLUE=9,
  LIGHT_GREEN=10,
  LIGHT_CYAN=11,
  LIGHT_RED=12,
  LIGHT_PURPLE=13,
  YELLOW=14,
  WHITE=15
} VGA_COLOR;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void init_vga(VGA_COLOR txt,VGA_COLOR bg);
void write_string(const char *string);
void clear_screen();
void screen_backspace();

#endif
