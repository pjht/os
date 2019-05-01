#include "../libc/string.h"

int main() {
  int x=17;
  char str[2];
  str[0]='h';
  str[1]='\0';
  char* vga=0xC00B8000;
  return x+strlen(str);
}
