#ifndef DEVBUF_H
#define DEVBUF_H

#include <stdint.h>

typedef struct {
  int buf[256];
  uint8_t rd;
  uint8_t wr;
} devbuf;

devbuf* devbuf_init();
void devbuf_add(char byte,devbuf* buf);
int devbuf_get(devbuf* buf);

#endif
