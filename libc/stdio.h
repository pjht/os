#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>
typedef struct {
  char* mntpnt;
  const char* path;
  uint32_t type;
  long pos;
  int eof;
} FILE;

#define NO_FD 0xFFFFFFFF

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3
#define EOF -1

extern uint32_t stdin;
extern uint32_t stdout;
extern uint32_t stderr;

uint32_t fopen(const char* filename,const char* mode);
int fgetc(FILE* stream);
int getc();
char* fgets(char* str,int count,FILE* stream);
size_t fread(void* buffer,size_t size,size_t count,FILE* stream);
int fputc(int c,FILE* stream);
int putc(int c);
int fputs(const char* s,FILE* stream);
int puts(const char* s);
int fprintf(FILE* stream,const char* format,...);
int printf(const char* format,...);
int fseek(FILE* stream,long offset,int origin);
long ftell(FILE* stream);
int fclose(FILE* file);

#endif
