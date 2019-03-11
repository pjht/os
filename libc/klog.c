#include <stdarg.h>
#include <stdio.h>

int vfprintf(FILE* stream,const char* format,va_list arg);

void klog(char* level,char* s,...) {
  if (stdout!=NULL) {
    va_list arg;
    va_start(arg,s);
    printf("[%s] ",level);
    vfprintf(stdout,s,arg);
    printf("\n");
    va_end(arg);
  }
}
