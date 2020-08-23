/**
 * \file
*/
#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#define SEEK_CUR 1 //!< Seek postition is relative to current position im file
#define SEEK_END 2 //!< Seek position is relative to end of file
#define SEEK_SET 3 //!< Seek position is relative to beginning of file
#define EOF -1 //!< Represents EOF

/**
 * Represents an open file
*/
typedef struct {
  pid_t fs_pid; //!< PID of the filesytem drive for this file
  void* fs_data; //!< Pointer given by the filesytem driver to its info about the file.
  int pos; //!< Positiom in the file
} FILE;

extern FILE* stdin; //!< Standard input
extern FILE* stdout; //!< Standard output
extern FILE* stderr; //!< Standard error

/**
 * Opens a file with the specified mode
 * \param filename The path to the file to open
 * \param mode The mode to open the file with (r,w,a,r+,w+,a+)
 * \returns NULL if the file could not be opened, or a pointer to a FILE struct if the opening was sucessful.
*/
FILE* fopen(char* filename,char* mode);

/**
 * Gets a character from a file
 * \param stream The file to read from
 * \returns the read character, or EOF if the read fails
*/
int fgetc(FILE* stream);

/**
 * Gets a character from a file
 * \param stream The file to read from
 * \returns the read character, or EOF if the read fails
*/
int getc(FILE* stream);

/**
 * Gets a newline delimeted string from a file
 * \param str The buffer to read into
 * \param count The maximum size of the string to read
 * \param stream The file to read from
 * \returns the buffer
*/
char* fgets(char* str,int count,FILE* stream);

/**
 * Gets a newline delimeted string from stdin
 * \param str The buffer to read into
 * \returns the buffer
 * \deprecated Gets is unsafe and vulnerable to a buffer overflow, as there is no bounds checking on the buffer.
*/
char* gets(char* str);

/**
 * Reads from a file
 * \param buffer The buffer to read into
 * \param size The size of each elemnt to read
 * \param count The number of elements to read
 * \param stream The file to read from
 * \returns the number of bytes read, or 0 on failure
*/
size_t fread(void* buffer,size_t size,size_t count,FILE* stream);

/**
 * Writes a character to a file
 * \param c The character to write
 * \param stream The stream to write to
 * \returns the written character, or EOF on failure
*/
int fputc(int c,FILE* stream);

/**
 * Writes a character to a file
 * \param c The character to write
 * \param stream The stream to write to
 * \returns the written character, or EOF on failure
*/
int putc(int c,FILE* stream);

/**
 * Writes a string to a file
 * \param s The string to write
 * \param stream The file to write to
 * \returns 0 on success, EOF on failure
*/
int fputs(const char* s,FILE* stream);

/**
 * Writes to a file
 * \param buffer_ptr The buffer to write
 * \param size The size of each elemnt to write
 * \param count The number of elements to write
 * \param stream The file to write to
 * \returns the number of bytes written, or 0 on failure
*/
size_t fwrite(void* buffer_ptr,size_t size,size_t count,FILE* stream);

/** 
 * Writes a string followed by a newline to stdin
 * \param s The string to write
 * \returns 0 on success, EOF on failure
*/
int puts(const char* s);

/**
 * Format a string and writte it to a file
 * \param stream The fike to write to
 * \param format The format string
 * \param ... The arguments for the format string
 * \returns The number of bytes written
*/
int fprintf(FILE* stream,const char* format,...);

/**
 * Format a string and writte it to a file
 * \param stream The fike to write to
 * \param format The format string
 * \param arg The arguments for the format string
 * \returns The number of bytes written
*/
int vfprintf(FILE* stream,const char* format,va_list arg);

/**
 * Format a string and write it to stdout
 * \param format The format string
 * \param ... The arguments for the format string
 * \returns The number of bytes written
*/
int printf(const char* format,...);

/**
 * Seeks to a position in a file
 * \param stream The file to seek in
 * \param offset The offset to seek to
 * \param origin The origin of seeking (SEEK_CUR,SEEK_END,SEEK_SET)
 * \returns 0 on success, -1 on failure
*/
int fseek(FILE* stream,long offset,int origin);

/**
 * Returns the postion in a file
 * \param stream The file to get the position in
 * \returns the position on success, -1 on failure
*/ 
long ftell(FILE* stream);

/**
 * Closes a file
 * \param file The file to close
 * \returns 0 on success, EOF on failure
*/
int fclose(FILE* file);

/**
 * Returns the EOF indicator for a file
 * \param stream the file to check
 * \returns the EOF indicator
*/
int feof(FILE* stream);

/**
 * Returns the error indicator for a file
 * \param stream the file to check
 * \returns the error indicator
*/
int ferror(FILE* stream);

/** Flushes the buffer on a file
 * \param stream The file to flush the buffer of
 * \returns 0 on success, EOF on failure
*/
int fflush(FILE *stream); 

/**
 * Assigns buffering to a file
 * 
 * When the buffer is not a null pointer, the buffer type is set to fully bufferd, and the buffer size is assumed to be at least BUFSIZ.
 * 
 * When the buffer is a null pointer, buffering is removed.
 * \param stream The file to assign buffering to
 * \param buf The buffer to use
*/
void setbuf(FILE *restrict stream, char *restrict buf);

/**
 * Registers a file system type with the VFS
 * \param name The name of the type to register
 * \param pid The PID of the filesytem driver
*/
void register_fs(const char* name,pid_t pid);

/**
 * Mounts a filesystem
 * \param file The file to mount
 * \param type The type of the filesystem to mount
 * \param path The path to mount the filesytem at
 * \returns 0 on success, 1 on failure
*/
int mount(char* file,char* type,char* path);

#endif
