
#include "../isr.h"
#include "../serial.h"
#include <cpu/ports.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#define SERIAL_LINE_ENABLE_DLAB 0x80

static char configured[]={0,0,0,0};

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


void serial_init() {
  port_byte_out(scratch_port(0),0xaa);
  if (port_byte_in(scratch_port(0))==0xaa) {
    configure(0,9600);
    configured[0]=1;
  }
}

static void serial_putc(char c) {
  if (c=='\n') {
    while (!is_transmit_fifo_empty(0)) continue;
    port_byte_out(data_port(0),'\r');
    while (!is_transmit_fifo_empty(0)) continue;
    port_byte_out(data_port(0),'\n');
  } else {
    while (!is_transmit_fifo_empty(0)) continue;
    port_byte_out(data_port(0),c);
  }
}

void serial_write_string(const char* s) {
  if (!configured[0]) return;
  for (int i=0;s[i]!='\0';i++) {
    serial_putc(s[i]);
  }
}

void serial_printf(const char* format,...) {
  va_list arg;
  va_start(arg,format);
  for(;*format!='\0';format++) {
    if(*format!='%') {
      serial_putc(*format);
      continue;
    }
    format++;
    switch(*format) {
      case 'c': {
        int i=va_arg(arg,int);
        serial_putc(i);
        break;
      }
      case 'd': {
        int i=va_arg(arg,int); 		//Fetch Decimal/Integer argument
        if(i<0) {
          i=-i;
          serial_putc('-');
        }
        char str[11];
        int_to_ascii(i,str);
        serial_write_string(str);
        break;
      }
      // case 'o': {
      //   int i=va_arg(arg,unsigned int); //Fetch Octal representation
      // 	puts(convert(i,8));
      // 	break;
      // }
      case 's': {
        char* s=va_arg(arg,char*);
        serial_write_string(s);
        break;
      }
      case 'x': {
        uint32_t i=va_arg(arg,uint32_t);
        char str[11];
        str[0]='\0';
        hex_to_ascii(i,str);
        serial_write_string(str);
        break;
      }
    }
  }
}
