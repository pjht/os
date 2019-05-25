#ifndef vga_H
#define vga_H

typedef enum {
  VGA_BLACK=0,
  VGA_BLUE=1,
  VGA_GREEN=2,
  VGA_CYAN=3,
  VGA_RED=4,
  VGA_PURPLE=5,
  VGA_BROWN=6,
  VGA_GRAY=7,
  VGA_DARK_GRAY=8,
  VGA_LIGHT_BLUE=9,
  VGA_LIGHT_GREEN=10,
  VGA_LIGHT_CYAN=11,
  VGA_LIGHT_RED=12,
  VGA_LIGHT_PURPLE=13,
  VGA_YELLOW=14,
  VGA_WHITE=15
} vga_colors;


void vga_init();
void vga_write_string(const char *string);
void vga_clear();
void vga_backspace();

#endif
