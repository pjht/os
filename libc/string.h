#ifndef STRING_H
#define STRING_H
#include <stddef.h>

void* memcpy(void* source,const void* dest,size_t len);
void* memset(void* dest,int val,size_t len);
int strcmp(const char* s1,const char* s2);
size_t strlen(const char* str);
char* strcpy(char* dest,const char* src);


char* strrev(char *str);
void int_to_ascii(int n,char* str);
void hex_to_ascii(int n, char* str);
void append(char* s, char n);
void backspace(char* s);
#endif
