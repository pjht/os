#include "../../cpu/i386/ports.h"
#include "../../cpu/i386/isr.h"
#include "../serial.h"
#include "../../libc/stdio.h"
#include "../../libc/string.h"
#include "../../libc/devbuf.h"
#include "../../fs/devfs.h"
#include "../../kernel/klog.h"
#include <stdint.h>

#define SERIAL_LINE_ENABLE_DLAB 0x80

devbuf* bufs[4];

int serial_data_port(int com) {
  switch (com) {
    case 0: return 0x3f8;
    case 1: return 0x2f8;
    case 2: return 0x3e8;
    case 3: return 0x2e8;
  }
  return 0;
}

int serial_int_port(int com) {
  switch (com) {
    case 0: return 0x3f9;
    case 1: return 0x2f9;
    case 2: return 0x3e9;
    case 3: return 0x2e9;
  }
  return 0;
}


int serial_fifo_port(int com) {
  switch (com) {
    case 0: return 0x3fa;
    case 1: return 0x2fa;
    case 2: return 0x3ea;
    case 3: return 0x2ea;
  }
  return 0;
}
int serial_line_cmd_port(int com) {
  switch (com) {
    case 0: return 0x3fb;
    case 1: return 0x2fb;
    case 2: return 0x3eb;
    case 3: return 0x2eb;
  }
  return 0;
}
int serial_modem_port(int com) {
  switch (com) {
    case 0: return 0x3fc;
    case 1: return 0x2fc;
    case 2: return 0x3ec;
    case 3: return 0x2ec;
  }
  return 0;
}
int serial_line_stat_port(int com) {
  switch (com) {
    case 0: return 0x3fd;
    case 1: return 0x2fd;
    case 2: return 0x3ed;
    case 3: return 0x2ed;
  }
  return 0;
}
int serial_scratch_port(int com) {
  switch (com) {
    case 0: return 0x3ff;
    case 1: return 0x2ff;
    case 2: return 0x3ef;
    case 3: return 0x2ef;
  }
  return 0;
}

void serial_configure_baud_rate(uint32_t divisor,int com) {
    port_byte_out(serial_line_cmd_port(com),SERIAL_LINE_ENABLE_DLAB);
    port_byte_out(serial_data_port(com),(divisor>>8)&0xFF);
    port_byte_out(serial_data_port(com),divisor&0xFF);
}

int serial_is_transmit_fifo_empty(int com) {
    return port_byte_in(serial_line_stat_port(com))&0x20;
}

void serial_configure(uint32_t com, uint32_t rate) {
  serial_configure_baud_rate(115200/rate,com);
  port_byte_out(serial_line_cmd_port(com),0x03);
  port_byte_out(serial_fifo_port(com),0xC7);
  port_byte_out(serial_modem_port(com),0x03);
  port_byte_out(serial_int_port(com),0x01);
}

int serial_dev_drv(char* filename,int c,long pos,char wr) {
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
    while (!serial_is_transmit_fifo_empty(com)) continue;
    port_byte_out(serial_data_port(com),c);
    return 0;
  } else {
    return devbuf_get(bufs[com]);
  }
}

void serial_int_handler_1(registers_t regs) {
  char data=port_byte_in(serial_data_port(0));
  if (data=='\r') {
    data='\n';
  }
  devbuf_add(data,bufs[0]);
}

void serial_int_handler_2(registers_t regs) {
  devbuf_add(port_byte_in(serial_data_port(1)),bufs[1]);
}


void serial_init() {
  klog("INFO","Scanning for serial ports");
  for (int i=0;i<2;i++) {
    port_byte_out(serial_scratch_port(i),0xaa);
    if (port_byte_in(serial_scratch_port(i))==0xaa) {
      klog("INFO","Found COM%d",i+1);
      bufs[i]=devbuf_init();
      switch (i) {
        case 0:
          register_interrupt_handler(IRQ4,serial_int_handler_1);
          serial_configure(0,9600);
          add_dev(serial_dev_drv,"ttyS0");
        case 1:
          register_interrupt_handler(IRQ3,serial_int_handler_2);
          serial_configure(1,9600);
          add_dev(serial_dev_drv,"ttyS1");
      }
    }
  }
}
