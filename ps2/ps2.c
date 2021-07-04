#include <cpu/ports.h>
#include <stdint.h>
#include <stdio.h>
#include "ps2.h"

void ps2_send_cmd(uint8_t cmd) {
  // serial_print("Writing ");
  // uint8_t str[64];
  // hex_to_ascii(cmd&0xFF,str);
  // serial_print(str);
  // serial_print(" to PS/2 command port\n");
  while (port_byte_in(PS2_STAT_CMD)&PS2_STAT_INP_BUF);
  port_byte_out(PS2_STAT_CMD,cmd);
}

uint8_t ps2_read_data() {
  while (!(port_byte_in(PS2_STAT_CMD)&PS2_STAT_OUT_BUF));
  uint8_t byte=port_byte_in(PS2_DATA);
  // serial_print("Read ");
  // uint8_t str[64];
  // hex_to_ascii(byte&0xFF,str);
  // serial_print(str);
  // serial_print(" from PS/2 data port\n");
  return byte;
}

void ps2_write_data(uint8_t byte) {
  // serial_print("Writing ");
  // uint8_t str[64];
  // hex_to_ascii((int)byte&0xFF,str);
  // serial_print(str);
  // serial_print(" to PS/2 data port\n");
  while (port_byte_in(PS2_STAT_CMD)&PS2_STAT_INP_BUF);
  port_byte_out(PS2_DATA,byte);
}

void ps2_write_data_to_device(int port,uint8_t data) {
  if (port==2) {
    ps2_send_cmd(0xD4);
  }
  ps2_write_data(data);
}

uint8_t ps2_send_cmd_to_device(int port,uint8_t cmd) {
  while (1) {
    ps2_write_data_to_device(port,cmd);
    uint8_t resp=ps2_read_data();
    if (cmd==0xEE && resp==0xEE) {
      return 1;
    }
    if (resp==0xFA || resp==0xAA) {
      return 1;
    }
    if (resp==0xFC || resp==0xFD) {
      return 0;
    }
  }
}

uint8_t ps2_send_cmd_w_data_to_device(int port,uint8_t cmd,uint8_t data) {
  while (1) {
    ps2_write_data_to_device(port,cmd);
    ps2_write_data_to_device(port,data);
    uint8_t resp=ps2_read_data();
    if (cmd==0xEE && resp==0xEE) {
      return 1;
    }
    if (resp==0xFA || resp==0xAA) {
      return 1;
    }
    if (resp==0xFC || resp==0xFD) {
      return 0;
    }
  }
}
