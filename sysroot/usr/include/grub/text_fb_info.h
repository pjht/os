#ifndef GRUB_TEXT_FB_INFO_H
#define GRUB_TEXT_FB_INFO_H

#include <stdint.h>

typedef struct {
  char* address;
  uint32_t width;
  uint32_t height;
} text_fb_info;

#endif
