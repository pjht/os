#include <devbuf.h>
#include <stdlib.h>
#include <stdio.h>

devbuf* devbuf_init() {
  devbuf* buf=malloc(sizeof(devbuf));
  buf->rd=0;
  buf->wr=0;
  for (int i=0;i<256;i++) {
    buf->buf[i]=EOF;
  }
  return buf;
}

void devbuf_add(char byte,devbuf* buf) {
  buf->buf[buf->wr]=byte;
  buf->wr++;
  if (buf->wr==buf->rd) {
    buf->wr--;
  }
}

int devbuf_get(devbuf* buf) {
  if (buf->buf[buf->rd]==-1) {
    buf->rd++;
    if (buf->buf[buf->rd]==-1) {
      buf->rd--;
      while (buf->buf[buf->rd]==-1);
    }
  }
  int data=buf->buf[buf->rd];
  buf->buf[buf->rd]=-1;
  buf->rd++;
  if (buf->rd>buf->wr) {
    buf->rd=buf->wr-1;
  }
  return data;
}
