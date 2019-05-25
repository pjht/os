#include "../../cpu/i386/ports.h"
#include "../../cpu/i386/isr.h"
#include "../serial.h"
#include <stdio.h>
#include <string.h>
#include <devbuf.h>
#include "../vga.h"
#include "../../fs/devfs.h"
#include <klog.h>
#include <stdint.h>

#define SERIAL_LINE_ENABLE_DLAB 0x80

static devbuf* bufs[4];

static int data_port(int com) {
  switch (com) {
    case 0: return 0x3f8;
    case 1: return 0x2f8;
    case 2: return 0x3e8;
    case 3: return 0x2e8;
  }
  return 0;
}

static int int_port(int com) {
  switch (com) {
    case 0: return 0x3f9;
    case 1: return 0x2f9;
    case 2: return 0x3e9;
    case 3: return 0x2e9;
  }
  return 0;
}


static int fifo_port(int com) {
  switch (com) {
    case 0: return 0x3fa;
    case 1: return 0x2fa;
    case 2: return 0x3ea;
    case 3: return 0x2ea;
  }
  return 0;
}
static int line_cmd_port(int com) {
  switch (com) {
    case 0: return 0x3fb;
    case 1: return 0x2fb;
    case 2: return 0x3eb;
    case 3: return 0x2eb;
  }
  return 0;
}
static int modem_port(int com) {
  switch (com) {
    case 0: return 0x3fc;
    case 1: return 0x2fc;
    case 2: return 0x3ec;
    case 3: return 0x2ec;
  }
  return 0;
}
static int line_stat_port(int com) {
  switch (com) {
    case 0: return 0x3fd;
    case 1: return 0x2fd;
    case 2: return 0x3ed;
    case 3: return 0x2ed;
  }
  return 0;
}
static int scratch_port(int com) {
  switch (com) {
    case 0: return 0x3ff;
    case 1: return 0x2ff;
    case 2: return 0x3ef;
    case 3: return 0x2ef;
  }
  return 0;
}

static void configure_baud_rate(uint32_t divisor,int com) {
    port_byte_out(line_cmd_port(com),SERIAL_LINE_ENABLE_DLAB);
    port_byte_out(data_port(com),(divisor>>8)&0xFF);
    port_byte_out(data_port(com),divisor&0xFF);
}

static int is_transmit_fifo_empty(int com) {
    return port_byte_in(line_stat_port(com))&0x20;
}

static void configure(uint32_t com, uint32_t rate) {
  configure_baud_rate(115200/rate,com);
  port_byte_out(line_cmd_port(com),0x03);
  port_byte_out(fifo_port(com),0xC7);
  port_byte_out(modem_port(com),0x03);
  port_byte_out(int_port(com),0x01);
}

static int drv(char* filename,int c,long pos,char wr) {
  int com;
  switch (filename[4]) {
    case '0':
      com=0;
      break;
    case '1':
      com=1;
      break;
    case '2':
      com=2;
      break;
    case '3':
      com=3;
      break;
  }
  if (wr) {
    while (!is_transmit_fifo_empty(com)) continue;
    port_byte_out(data_port(com),c);
    return 0;
  } else {
    return devbuf_get(bufs[com]);
  }
}

void serial_int_handler_1(registers_t regs) {
  char data=port_byte_in(data_port(0));
  if (data=='\r') {
    data='\n';
  }
  devbuf_add(data,bufs[0]);
}

void serial_int_handler_2(registers_t regs) {
  devbuf_add(port_byte_in(data_port(1)),bufs[1]);
}


void serial_init() {
  klog("INFO","Scanning for serial ports");
  for (int i=0;i<2;i++) {
    port_byte_out(scratch_port(i),0xaa);
    if (port_byte_in(scratch_port(i))==0xaa) {
      klog("INFO","Found COM%d",i+1);
      bufs[i]=devbuf_init();
      switch (i) {
        case 0:
          isr_register_handler(IRQ4,serial_int_handler_1);
          configure(0,9600);
          devfs_add(drv,"ttyS0");
          break;
        case 1:
          isr_register_handler(IRQ3,serial_int_handler_2);
          configure(1,9600);
          devfs_add(drv,"ttyS1");
      }
    }
  }
}
