#ifndef PS2_H
#define PS2_H

#include <stdint.h>

#define PS2_DATA 0x60
#define PS2_STAT_CMD 0x64
#define PS2_STAT_OUT_BUF 0x1
#define PS2_STAT_INP_BUF 0x2
#define PS2_KBD_TYPE 0x1

void ps2_send_cmd(uint8_t cmd);
uint8_t ps2_read_data();
void ps2_write_data(uint8_t byte);
void ps2_write_data_to_device(int port,uint8_t data);
uint8_t ps2_send_cmd_to_device(int port,uint8_t cmd);
uint8_t ps2_send_cmd_w_data_to_device(int port,uint8_t cmd,uint8_t data);

#endif
