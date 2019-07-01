#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define FILE uint32_t //We're using pointers to FILE even though it's a uint32_t so we can expand to a struct if needed
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3
#define EOF -1

#define stdin 0
#define stdout 1
#define stderr 2


FILE* fopen(char* filename,char* mode);
int fgetc(FILE* stream);
int getc();
char* fgets(char* str,int count,FILE* stream);
size_t fread(void* buffer,size_t size,size_t count,FILE* stream);
int fputc(int c,FILE* stream);
int putc(int c);
int fputs(const char* s,FILE* stream);
size_t fwrite(void* buffer_ptr,size_t size,size_t count,FILE* stream);
int puts(const char* s);
int fprintf(FILE* stream,const char* format,...);
int vfprintf(FILE* stream,const char* format,va_list arg);
int printf(const char* format,...);
int fseek(FILE* stream,long offset,int origin);
long ftell(FILE* stream);
int fclose(FILE* file);
int feof(FILE* stream);
int ferror(FILE* stream);

#endif
