#include "serial.h"
#include <stdint.h>
#include <stdarg.h>

void serial_write_string(const char* s) {
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
