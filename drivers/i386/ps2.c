#include "../../cpu/i386/ports.h"
#include "../../kernel/klog.h"
#include "../screen.h"
#include "ps2/keyboard.h"
#include <stdint.h>
#define PS2_DATA 0x60
#define PS2_STAT_CMD 0x64
#define PS2_STAT_OUT_BUF 0x1
#define PS2_STAT_INP_BUF 0x2
#define PS2_KBD_TYPE 0x1
char ps2_ports_ok[2]={0,0};
char ps2_dev_types[2]={0,0};

void ps2_send_cmd(char cmd) {
  while (port_byte_in(PS2_STAT_CMD)&PS2_STAT_INP_BUF);
  port_byte_out(PS2_STAT_CMD,cmd);
}

char ps2_read_data() {
  while (!(port_byte_in(PS2_STAT_CMD)&PS2_STAT_OUT_BUF));
  return port_byte_in(PS2_DATA);
}

void ps2_write_data(char byte) {
  while (port_byte_in(PS2_STAT_CMD)&PS2_STAT_INP_BUF);
  port_byte_out(PS2_DATA,byte);
}

void ps2_write_data_to_device(int port,char data) {
  if (port==2) {
    ps2_send_cmd(0xD4);
  }
  ps2_write_data(data);
}

char ps2_send_cmd_to_device(int port,char cmd) {
  while (1) {
      ps2_write_data_to_device(port,cmd);
      uint8_t resp=ps2_read_data();
      if (cmd==0xEE && resp==0xEE) {
        return 1;
      }
      if (resp==0xFA) {
        return 1;
      }
      if (resp==0xFC || resp==0xFD) {
        return 0;
      }
  }
}

char ps2_send_cmd_w_data_to_device(int port,char cmd,char data) {
  while (1) {
      ps2_write_data_to_device(port,cmd);
      ps2_write_data_to_device(port,data);
      uint8_t resp=ps2_read_data();
      if (cmd==0xEE && resp==0xEE) {
        return 1;
      }
      if (resp==0xFA) {
        return 1;
      }
      if (resp==0xFC || resp==0xFD) {
        return 0;
      }
  }
}


void ps2_init() {
  ps2_send_cmd(0xAA);
  char self_test=ps2_read_data();
  if (self_test!=0x55) {
    klog("INFO","Cannot initialise PS/2 controller");
    return;
  }
  char is_dual;
  ps2_send_cmd(0xAD);
  ps2_send_cmd(0xA7);
  port_byte_in(PS2_DATA);
  ps2_send_cmd(0x20);
  char cont_cfg=ps2_read_data();
  cont_cfg=cont_cfg&0xBC;
  ps2_send_cmd(0x60);
  ps2_write_data(cont_cfg);
  if (cont_cfg&0x20) {
    is_dual=0;
  } else {
    ps2_send_cmd(0xA8);
    ps2_send_cmd(0x20);
    cont_cfg=ps2_read_data();
    is_dual=(cont_cfg&0x20)!=0;
    ps2_send_cmd(0xA7);
  }
  ps2_send_cmd(0xAB);
  if (ps2_read_data()==0) {
    klog("INFO","First PS/2 port OK");
    ps2_ports_ok[0]=1;
  } else {
    klog("INFO","Second PS/2 port OK");
  }
  if (is_dual) {
    ps2_send_cmd(0xA9);
    if (ps2_read_data()==0) {
      klog("INFO","Second PS/2 port OK");
      ps2_ports_ok[1]=1;
    } else {
      klog("INFO","Second PS/2 port not OK");
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
    ps2_ports_ok[0]=ps2_send_cmd_to_device(1,0xFF);
  }
  if (ps2_ports_ok[1]) {
    ps2_send_cmd(0xA8);
    ps2_send_cmd_to_device(2,0xF5);
    port_byte_in(PS2_DATA);
    ps2_send_cmd(0x20);
    cont_cfg=ps2_read_data();
    cont_cfg=cont_cfg&0x2;
    ps2_send_cmd(0x60);
    ps2_write_data(cont_cfg);
    ps2_ports_ok[1]=ps2_send_cmd_to_device(2,0xFF);
  }
  if (ps2_ports_ok[0]) {
    ps2_send_cmd_to_device(1,0xF2);
    uint8_t first_byte=ps2_read_data();
    if (first_byte==0xAB) {
      uint8_t sec_byte=ps2_read_data();
      if (sec_byte==0x41 || sec_byte==0xC1 || sec_byte==0x83) {
        klog("INFO","Keyboard on PS/2 port 1");
        init_keyboard(1);
      }
    }
    if (first_byte==0) {
      klog("INFO","Mouse on PS/2 port 1");
    }
    if (first_byte==3) {
      klog("INFO","Mouse w/scroll wheel on PS/2 port 1");
    }
    if (first_byte==4) {
      klog("INFO","5 button mouse on PS/2 port 1");
    }
    port_byte_in(PS2_DATA);
  }
  if (ps2_ports_ok[1]) {
    ps2_send_cmd_to_device(2,0xF2);
    uint8_t first_byte=ps2_read_data();
    if (first_byte==0xAB) {
      uint8_t sec_byte=ps2_read_data();
      if (sec_byte==0x83) {
        klog("INFO","Keyboard on PS/2 port 2");
        init_keyboard(2);
      }
    }
    if (first_byte==0) {
      klog("INFO","Mouse on PS/2 port 2");
    }
    if (first_byte==3) {
      klog("INFO","Mouse w/scroll wheel on PS/2 port 2");
    }
    if (first_byte==4) {
      klog("INFO","5 button mouse on PS/2 port 2");
    }
    port_byte_in(PS2_DATA);
  }
  klog("INFO","Finished initializing PS/2 driver");
}



char port_ok(char port) {
  return 0;
}
