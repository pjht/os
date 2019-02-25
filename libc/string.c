#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void* memcpy(void* dest_ptr,const void* source_ptr,size_t len) {
    char* source=(char*)source_ptr;
    char* dest=(char*)dest_ptr;
    for(size_t i=0;i<len;i++) {
      dest[i]=source[i];
    }
    return dest_ptr;
}

void* memset(void *dest_ptr,int val,size_t len) {
    char* dest=(char*)dest_ptr;
    for (size_t i=0;i<len;i++){
      dest[i]=(char)val;
    }
    return dest_ptr;
}

int strcmp(const char* s1,const char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

size_t strlen(const char* str) {
  size_t i;
  for (i=0;str[i]!='\0';i++);
  return i;
}

char* strcpy(char* dest,const char* src) {
  size_t i;
  for (i=0;i<strlen(src);i++) {
    dest[i]=src[i];
  }
  dest[i]='\0';
  return dest;
}

char* strrev(char* str) {
    char chr;
    int i,j;
    for (i=0,j=strlen(str)-1;i<j;i++,j--) {
      chr=str[i];
      str[i]=str[j];
      str[j]=chr;
    }
    return str;
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
    strrev(str);
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

char* strtok_str=NULL;
size_t strtok_index;

char strtok_delim_check(const char* delim) {
  for (size_t i=0;i<strlen(delim);i++) {
    if (strtok_str[strtok_index]==delim[i]||strtok_str[strtok_index]=='\0') {
      return 0;
    }
  }
  return 1;
}

char* strtok(char* str, const char* delim) {
  if (str!=NULL) {
    strtok_str=str;
    strtok_index=0;
  }
  if (!strtok_str || strtok_index>strlen(strtok_str)) {
    return NULL;
  }
  char* tok=malloc(sizeof(char)*32);
  tok[0]='\0';
  size_t max_len=32;
  for (;strtok_delim_check(delim);strtok_index++) {
    if (strlen(tok)+1==max_len) {
      tok=realloc(tok,sizeof(char)*(max_len+32));
      max_len+=32;
    }
    append(tok,strtok_str[strtok_index]);
  }
  strtok_index++;
  return tok;
}
