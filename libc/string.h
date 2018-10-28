#ifndef STRING_H
#define STRING_H

int strlen(const char* str);
void reverse(char* str);
void int_to_ascii(int n,char* str);
void hex_to_ascii(int n, char* str);
void append(char* s, char n);
void backspace(char* s);
char* strcpy(char* dest,const char* src);
int strcmp(char* s1, char* s2);

#endif
