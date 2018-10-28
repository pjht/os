#include "string.h"

int strlen(const char* str) {
  int i;
  for (i=0;str[i]!='\0';i++) {
    continue;
  }
  return i;
}

void reverse(char* str) {
    char chr;
    int j;
    for (int i=0,j=strlen(str)-1;i<j;i++,j--) {
      chr=str[i];
      str[i]=str[j];
      str[j]=chr;
    }
}

void int_to_ascii(int n,char* str) {
    int i;
    int sign;
    if ((sign = n)<0) {
      n=-n;
    }
    i=0;
    do {
      str[i++]=n%10+'0';
    } while ((n /= 10) > 0);
    if (sign < 0) {
      str[i++] = '-';
    }
    str[i]='\0';
    reverse(str);
}

void hex_to_ascii(int n, char* str) {
    append(str, '0');
    append(str, 'x');
    char zeros = 0;

    unsigned int tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp > 0xA) append(str, tmp - 0xA + 'a');
        else append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');
}

void append(char* s, char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

void backspace(char* s) {
    int len = strlen(s);
    s[len-1] = '\0';
}

char* strcpy(char* dest,const char* src) {
  int i;
  for (i=0;i<strlen(src);i++) {
    dest[i]=src[i];
  }
  dest[i]='\0';
}

int strcmp(char* s1, char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}
