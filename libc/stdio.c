/**
 * \file
*/

#include <stdio.h>
#include <serdes.h>
#include <rpc.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <dbg.h>

FILE* stdin=NULL; //!< Standard input
FILE* stdout=NULL; //!< Standard output
FILE* stderr=NULL; //!< Standard error

/** 
 * Initialize stdio.
 * Must not be called by user code.
*/
void __stdio_init() {
}

FILE* fopen(char* filename,char* mode) {
  serdes_state state={0};
  serialize_str(filename,&state);
  void* retval=rpc_call(2,"open",state.buf,state.sizeorpos);
  free(state.buf);
  start_deserialize(retval,&state);
  int err=deserialize_int(&state);
  void* fs_data=deserialize_ptr(&state);
  pid_t fs_pid=deserialize_int(&state);
  rpc_deallocate_buf(retval,state.sizeorpos);
  if (err) {
    return NULL;
  } else {
    FILE* file=malloc(sizeof(FILE));
    file->fs_pid=fs_pid;
    file->fs_data=fs_data;
    file->pos=0;
    return file;
  }
}

/**
 * Writes a character to a file
 * \param c The character to write
 * \param stream The stream to write to
 * \returns the written character, or EOF on failure
*/
int putc(int c, FILE* stream) __attribute__ ((alias ("fputc")));

int fputc(int c, FILE* stream) {
  char str[]={c,'\0'};
  if (fputs(str,stream)==0) {
    return EOF;
  } else {
    return c;
  }
  return EOF;
}

/**
 * Gets a character from a file
 * \param stream The file to read from
 * \returns the read character, or EOF if the read fails
*/
int getc(FILE* stream) __attribute__ ((alias ("fgetc"))); 

int fgetc(FILE* stream) {
  char c[2];
  if (fgets(&c[0],1,stream)==NULL) {
    return EOF;
  } else {
    return c[0];
  }
  return EOF;
}

char* gets(char* s) {
  return fgets(s,INT_MAX,stdin);
}

char* fgets(char* str,int count,FILE* stream) {
  count=fread(str,1,count-1,stream)+1;
  if (count==0) {
    return NULL;
  }
  str[count]='\0';
  int newlinepos=-1;
  for (int i=0;i<(count-1);i++) {
    if (str[i]=='\n') {
      newlinepos=i;
      break;
    }
  }
  if (newlinepos) {
    stream->pos-=(count-1);
    stream->pos+=newlinepos;
  }
  return str;
}

size_t fread(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  serdes_state state={0};
  serialize_ptr(stream->fs_data,&state);
  serialize_int(size*count,&state);
  serialize_int(stream->pos,&state);
  void* retbuf=rpc_call(stream->fs_pid,"read",state.buf,state.sizeorpos);
  free(state.buf);
  state.buf=NULL;
  state.sizeorpos=0;
  start_deserialize(retbuf,&state);
  int bytes_read=deserialize_int(&state);
  if (bytes_read) {
    void* ary=deserialize_ary(bytes_read,&state);
    memcpy(buffer_ptr,ary,size*count);
  }
  rpc_deallocate_buf(retbuf,state.sizeorpos);
  stream->pos+=bytes_read;
  return bytes_read;
}

int puts(const char *s) {
  char* str=malloc(sizeof(char)*(strlen(s)+2));
  strcpy(str,s);
  str[strlen(s)]='\n';
  str[strlen(s)+1]='\0';
  int code=fputs(str,stdout);
  free(str);
  return code;
}

int fputs(const char* s, FILE* stream) {
  size_t retval=fwrite((void*)s,strlen(s),1,stream);
  if (retval==0) {
    return EOF;
  } else {
    return 0;
  }
}

size_t fwrite(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  serdes_state state={0};
  serialize_ptr(stream->fs_data,&state);
  serialize_int(size*count,&state);
  serialize_int(stream->pos,&state);
  serialize_ary(buffer_ptr,size*count,&state);
  void* retbuf=rpc_call(stream->fs_pid,"write",state.buf,state.sizeorpos);
  free(state.buf);
  start_deserialize(retbuf,&state);
  int bytes_wrote=deserialize_int(&state);
  rpc_deallocate_buf(retbuf,state.sizeorpos);
  stream->pos+=bytes_wrote;
  return bytes_wrote;
}

void register_fs(const char* name,pid_t pid) {
  serdes_state state={0};
  serial_print("libc register fs 1\n");
  serialize_str((char*)name,&state);
  serial_print("libc register fs 2\n");
  serialize_int(pid,&state);
  serial_print("libc register fs 3\n");
  rpc_call(2,"register_fs",state.buf,state.sizeorpos);
}

int mount(char* file,char* type,char* path) {
  serdes_state state={0};
  serialize_str(type,&state);
  serialize_str(file,&state);
  serialize_str(path,&state);
  int* err=rpc_call(2,"mount",state.buf,state.sizeorpos);
  return *err;
}

int vfprintf(FILE* stream,const char* format,va_list arg) {
  int c;
	for(;*format!='\0';format++) {
    if(*format!='%') {
  		c=fputc(*format,stream);
      continue;
  	}
    format++;
		switch(*format) {
			case 'c': {
        int i=va_arg(arg,int);
				c=fputc(i,stream);
        if (c==EOF) {
          return EOF;
        }
				break;
      }
			case 'd': {
        int i=va_arg(arg,int); 		//Fetch Decimal/Integer argument
				if(i<0) {
					i=-i;
					fputc('-',stream);
				}
        char str[11];
        int_to_ascii(i,str);
				c=fputs(str,stream);
        if (c==EOF) {
          return EOF;
        }
				break;
      }
			// case 'o': {
      //   int i=va_arg(arg,unsigned int); //Fetch Octal representation
			// 	puts(convert(i,8));
			// 	break;
      // }
			case 's': {
        char* s=va_arg(arg,char*);
				c=fputs(s,stream);
        if (c==EOF) {
          return EOF;
        }
				break;
      }
			case 'x': {
        unsigned int i=va_arg(arg, unsigned int);
        char str[11];
        str[0]='\0';
        hex_to_ascii(i,str);
				c=fputs(str,stream);
        if (c==EOF) {
          return EOF;
        }
				break;
      }
		}
	}
  return 1;
}

int fprintf(FILE* stream,const char* format,...) {
  va_list arg;
  int code;
  va_start(arg,format);
  code=vfprintf(stream,format,arg);
  va_end(arg);
  if (code) {
    return strlen(format);
  } else {
    return EOF;
  }
}

int printf(const char* format,...) {
  va_list arg;
  int code;
  va_start(arg,format);
  code=vfprintf(stdout,format,arg);
  va_end(arg);
  if (code) {
    return strlen(format);
  } else {
    return EOF;
  }
}

int fseek(FILE* stream,long offset,int origin) {
  switch (origin) {
  case SEEK_SET:
    stream->pos=offset;
    break;
  case SEEK_CUR:
    stream->pos+=offset;
    break;
  case SEEK_END:
    break;
  default:
    break;
  }
  return 0;
}
