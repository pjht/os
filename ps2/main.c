#include <stdio.h>
#include <stdint.h>
#include <cpu/ports.h>
#include "ps2.h"
#include "keyboard.h"
char ps2_ports_ok[2]={0,0};
char ps2_dev_types[2]={0,0};



int main() {
  stdout=fopen("/dev/vga","w");
  ps2_send_cmd(0xAA);
  uint8_t self_test=ps2_read_data();
  if (self_test!=0x55) {
    printf("[INFO] Cannot initialise PS/2 controller\n");
    return 0;
  }
  char is_dual;
  ps2_send_cmd(0xAD);
  ps2_send_cmd(0xA7);
  port_byte_in(PS2_DATA);
  ps2_send_cmd(0x20);
  uint8_t cont_cfg=ps2_read_data();
  cont_cfg=cont_cfg&0xBC;
  ps2_send_cmd(0x60);
  ps2_write_data(cont_cfg);
  if (cont_cfg&0x20) {
    is_dual=0;
  } else {
    ps2_send_cmd(0xA8);
    ps2_send_cmd(0x20);
    cont_cfg=ps2_read_data();
    is_dual=(cont_cfg&0x20)==0;
    ps2_send_cmd(0xA7);
  }
  ps2_send_cmd(0xAB);
  if (ps2_read_data()==0) {
    printf("[INFO] First PS/2 port OK\n");
    ps2_ports_ok[0]=1;
  } else {
    printf("[INFO] First PS/2 port not OK\n");
  }
  if (is_dual) {
    ps2_send_cmd(0xA9);
    if (ps2_read_data()==0) {
      printf("[INFO] Second PS/2 port OK\n");
      ps2_ports_ok[1]=1;
    } else {
      printf("[INFO] Second PS/2 port not OK\n");
    }
  }
  if (ps2_ports_ok[0]) {
    ps2_send_cmd(0xAE);
    ps2_send_cmd_to_device(1,0xF5);
    port_byte_in(PS2_DATA);
    ps2_send_cmd(0x20);
    cont_cfg=ps2_read_data();
    cont_cfg=cont_cfg|0x1;
    ps2_send_cmd(0x60);
    ps2_write_data(cont_cfg);
    //ps2_ports_ok[0]=ps2_send_cmd_to_device(1,0xFF);
  }
  if (ps2_ports_ok[1]) {
    ps2_send_cmd(0xA8);
    ps2_send_cmd_to_device(2,0xF5);
    port_byte_in(PS2_DATA);
    ps2_send_cmd(0x20);
    cont_cfg=ps2_read_data();
    cont_cfg=cont_cfg|0x2;
    ps2_send_cmd(0x60);
    ps2_write_data(cont_cfg);
    //ps2_ports_ok[1]=ps2_send_cmd_to_device(2,0xFF);
  }
  if (ps2_ports_ok[0]) {
    ps2_send_cmd_to_device(1,0xF2);
    uint8_t first_byte=ps2_read_data();
    if (first_byte==0xAB) {
      uint8_t sec_byte=ps2_read_data();
      if (sec_byte==0x41 || sec_byte==0xC1 || sec_byte==0x83) {
        printf("[INFO] Keyboard on PS/2 port 1\n");
        init_keyboard(1);
      }
    }
    if (first_byte==0) {
      printf("[INFO] Mouse on PS/2 port 1\n");
    }
    if (first_byte==3) {
      printf("[INFO] Mouse w/scroll wheel on PS/2 port 1\n");
    }
    if (first_byte==4) {
      printf("[INFO] 5 button mouse on PS/2 port 1\n");
    }
    port_byte_in(PS2_DATA);
  }
  if (ps2_ports_ok[1]) {
    ps2_send_cmd_to_device(2,0xF2);
    uint8_t first_byte=ps2_read_data();
    if (first_byte==0xAB) {
      uint8_t sec_byte=ps2_read_data();
      if (sec_byte==0x83) {
        printf("[INFO] Keyboard on PS/2 port 2\n");
        init_keyboard(2);
      }
    }
    if (first_byte==0) {
      printf("[INFO] Mouse on PS/2 port 2\n");
    }
    if (first_byte==3) {
      printf("[INFO] Mouse w/scroll wheel on PS/2 port 2\n");
    }
    if (first_byte==4) {
      printf("[INFO] 5 button mouse on PS/2 port 2\n");
    }
    port_byte_in(PS2_DATA);
  }
  printf("[INFO] PS/2 driver initialized\n");
  return 0;
}
