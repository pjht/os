#include "../../cpu/i386/ports.h"
#include "../serial.h"
#include <stdint.h>

#define SERIAL_DATA_PORT 0x3F8
#define SERIAL_FIFO_COMMAND_PORT 0x3FA
#define SERIAL_LINE_COMMAND_PORT 0x3FB
#define SERIAL_MODEM_COMMAND_PORT 0x3FC
#define SERIAL_LINE_STATUS_PORT 0x3FD

#define SERIAL_LINE_ENABLE_DLAB 0x80

void serial_configure_baud_rate(uint32_t divisor) {
    port_byte_out(SERIAL_LINE_COMMAND_PORT,SERIAL_LINE_ENABLE_DLAB);
    port_byte_out(SERIAL_DATA_PORT,(divisor>>8)&0xFF);
    port_byte_out(SERIAL_DATA_PORT,divisor&0xFF);
}

int serial_is_transmit_fifo_empty() {
    return port_byte_in(SERIAL_LINE_STATUS_PORT)&0x20;
}

void serial_configure(uint32_t com, uint32_t rate) {
  if (com!=1) {
    return;
  }
  serial_configure_baud_rate(115200/rate);
  port_byte_out(SERIAL_LINE_COMMAND_PORT,0x03);
  port_byte_out(SERIAL_FIFO_COMMAND_PORT,0xC7);
  port_byte_out(SERIAL_MODEM_COMMAND_PORT,0x03);
}

void serial_write_string(uint32_t com, char *str) {
  if (com!=1) {
    return;
  }
  while (*str!='\0') {
    while (!serial_is_transmit_fifo_empty()) continue;
    if (*str=='\n') {
      port_byte_out(SERIAL_DATA_PORT,'\r');
      port_byte_out(SERIAL_DATA_PORT,'\n');
    } else {
      port_byte_out(SERIAL_DATA_PORT,*str);
    }
    str++;
  }
}
