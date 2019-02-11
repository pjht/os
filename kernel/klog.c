#include <stdarg.h>
#include "vfs.h"
#include "../drivers/screen.h"

int vfprintf(FILE* stream,const char* format,va_list arg);

void klog(char* level,char* s,...) {
  if (stdout!=NO_FD) {
    va_list arg;
    va_start(arg,s);
    printf("[%s] ",level);
    vfprintf(stdout,s,arg);
    printf("\n");
    va_end(arg);
  } else {
    screen_write_string("[");
    screen_write_string(level);
    screen_write_string("] ");
    screen_write_string(s);
    screen_write_string("\n");
  }
}
