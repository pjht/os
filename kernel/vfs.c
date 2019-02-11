#include <stdint.h>
#include <stdarg.h>
#include "../libc/stdlib.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../cpu/tasking.h"
#include "vfs.h"
typedef struct _vfs_mapping_struct {
  char* mntpnt;
  uint32_t type;
  struct _vfs_mapping_struct* next;
} vfs_mapping;

#define MAX_FILES 512
#define BMAP_SZ (MAX_FILES/8)

char** drv_names;
fs_drv* drvs;
uint32_t max_drvs;
uint32_t next_drv_indx;
vfs_mapping* head_mapping;
vfs_mapping* tail_mapping;
char vfs_initialized=0;

uint32_t stdin=NO_FD;
uint32_t stdout=NO_FD;
uint32_t stderr=NO_FD;
FILE** files=NULL;
char* filebmap;

int vfsstrcmp(const char* s1,const char* s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    if (s1[i] == '\0') return 0;
    return s1[i] - s2[i];
}

char vfs_get_bmap_bit(uint32_t byte_idx,char offset) {
  char byte=filebmap[byte_idx];
  char bit=byte&(1<<offset);
  return bit>>offset;
}

void vfs_set_bmap_bit(uint32_t byte_idx,char offset,char bit) {
  char byte=filebmap[byte_idx];
  byte&=(bit<<offset);
  filebmap[byte_idx]=byte;
}

uint32_t get_fd() {
  for(int i=0;i<BMAP_SZ;i++) {
    for (int j=0;j<8;j++) {
      if (vfs_get_bmap_bit(i,j)==0) {
        uint32_t idx=i*8+j;
        vfs_set_bmap_bit(i,j,1);
        return idx;
      }
      yield();
    }
    yield();
  }
  return NO_FD;
}

void init_vfs() {
  drvs=malloc(sizeof(fs_drv)*32);
  drv_names=malloc(sizeof(fs_drv)*32);
  max_drvs=32;
  next_drv_indx=0;
  head_mapping=NULL;
  tail_mapping=NULL;
  filebmap=malloc(sizeof(char)*(BMAP_SZ));
  for(int i=0;i<BMAP_SZ;i++) {
    filebmap[i]=0;
  }
  files=malloc(sizeof(FILE*)*(MAX_FILES));
  vfs_initialized=1;
}

uint32_t register_fs(fs_drv drv,const char* type) {
  if (next_drv_indx==max_drvs) {
    drvs=realloc(drvs,sizeof(fs_drv)*(max_drvs+32));
    drv_names=realloc(drv_names,sizeof(char*)*(max_drvs+32));
    max_drvs+=32;
  }
  drvs[next_drv_indx]=drv;
  drv_names[next_drv_indx]=type;
  next_drv_indx++;
  return next_drv_indx-1;
}

char mount(char* mntpnt,char* dev,char* type) {
  uint32_t i;
  for (i=0;i<next_drv_indx;i++) {
    char* name=drv_names[i];
    if (strcmp(name,type)==0) {
      break;
    }
  }
  char ok=drvs[i](FSOP_MOUNT,NULL,mntpnt,dev);
  if (ok) {
    if (head_mapping==NULL) {
      vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
      mapping->mntpnt=malloc(sizeof(char)*(strlen(mntpnt)+1));
      strcpy(mapping->mntpnt,mntpnt);
      mapping->type=i;
      mapping->next=NULL;
      head_mapping=mapping;
      tail_mapping=mapping;
    } else {
      vfs_mapping* mapping=malloc(sizeof(vfs_mapping));
      mapping->mntpnt=malloc(sizeof(char)*(strlen(mntpnt)+1));
      strcpy(mapping->mntpnt,mntpnt);
      mapping->type=i;
      mapping->next=NULL;
      tail_mapping->next=mapping;
    }
    return 1;
  } else {
    return 0;
  }
}

uint32_t fopen(const char* filename,const char* mode) {
  char* msg=malloc(sizeof(char)*(6+strlen(mode)+1+strlen(filename)+1));
  strcpy("fopen ",msg);
  for (int i=0;i<strlen(mode);i++) {
    msg[6+i]=mode[i];
  }
  msg[6+strlen(mode)]=" ";
  for (int i=0;i<strlen(filename);i++) {
    msg[6+strlen(mode)+1+i]=filename[i];
  }
  msg[6+strlen(mode)+1+strlen(filename)+1]='\0';
  klog("INFO",msg);
  return NO_FD;
  send_msg(2,msg);
  yield();
  uint32_t sender;
  char* fd_str=get_msg(&sender);
  while (fd_str==NULL) {
    yield();
    char* fd_str=get_msg(&sender);
  }
  uint32_t fd;
  uint32_t number=0;
  for (int i=0;i<strlen(fd_str);i++) {
    number+=(fd_str[i]-0x30);
    number*=10;
  }
  return number;
}

int fgetc(FILE* stream) {
  int c;
  drvs[stream->type](FSOP_GETC,stream,&c,NULL);
  return c;
}

int getc() {
  return fgetc(files[stdin]);
}

char* fgets(char* str,int count,FILE* stream) {
  int i;
  for (i=0;i<count-1;i++) {
    char c=fgetc(stream);
    if (c==EOF) {
      break;
    } else if (c=='\n') {
      str[i]=c;
      i++;
      break;
    }
    str[i]=c;
  }
  str[i]='\0';
  return str;
}

size_t fread(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  char* buffer=(char*)buffer_ptr;
  size_t bytes=size*count;
  for (size_t i=0;i<bytes;i++) {
    int c=fgetc(stream);
    if (c==EOF) {
      return (size_t)((double)i/size);
    }
    buffer[i]=c;
  }
  return count;
}

int fputc(int c,FILE* stream) {
  char ok=drvs[stream->type](FSOP_PUTC,stream,&c,NULL);
  if (ok) {
    return c;
  } else {
    return EOF;
  }
}

int putc(int c) {
  return fputc(c,files[stdout]);
}

int fputs(const char* s,FILE* stream) {
  size_t len=strlen(s);
  for (size_t i=0;i<len;i++) {
    fputc(s[i],stream);
  }
  return 1;
}

int puts(const char* s) {
  return fputs(s,files[stdout]);
}

int vfprintf(FILE* stream,const char* format,va_list arg) {
	for(;*format!='\0';format++) {
    if(*format!='%') {
  		fputc(*format,stream);
      continue;
  	}
    format++;
		switch(*format) {
			case 'c': {
        int i=va_arg(arg,int);
				fputc(i,stream);
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
				fputs(str,stream);
				break;
      }
			// case 'o': {
      //   int i=va_arg(arg,unsigned int); //Fetch Octal representation
			// 	puts(convert(i,8));
			// 	break;
      // }
			case 's': {
        char* s=va_arg(arg,char*);
				fputs(s,stream);
				break;
      }
			case 'x': {
        uint32_t i=va_arg(arg,uint32_t);
        char str[11];
        str[0]='\0';
        hex_to_ascii(i,str);
				fputs(str,stream);
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
  code=vfprintf(files[stdout],format,arg);
  va_end(arg);
  if (code) {
    return strlen(format);
  } else {
    return EOF;
  }
}

int fseek(FILE* stream,long offset,int origin) {
  if (origin==SEEK_SET) {
    stream->pos=offset;
  }
  if (origin==SEEK_CUR) {
    stream->pos+=offset;
  }
  return 0;
}

long ftell(FILE* stream) {
  return stream->pos;
}

int fclose(FILE* stream) {
  drvs[stream->type](FSOP_CLOSE,stream,NULL,NULL);
  free(stream);
  return 0;
}

void vfs_task() {
  init_vfs();
  while (1) {
    uint32_t sender;
    char* msg=get_msg(&sender);
    if (msg) {
      char* cmd=strtok(msg," ");
      if (strcmp(cmd,"fopen")==0) {
        int fd=get_fd();
        if (fd==NO_FD) {
          send_msg(sender,"4294967295");
        }
        char* mode=strtok(NULL," ");
        char* filename=strtok(NULL,"");
        vfs_mapping* mnt=head_mapping;
        vfs_mapping* mntpnt=NULL;
        const char* path;
        uint32_t mntpnt_len=0;
        for (;mnt!=NULL;mnt=mnt->next) {
          char* root=mnt->mntpnt;
          if (strlen(root)>mntpnt_len) {
            if (vfsstrcmp(root,filename)==0) {
              mntpnt=mnt;
              mntpnt_len=strlen(root);
            }
          }
        }
        if (mntpnt) {
          path=filename+mntpnt_len;
          FILE* stream=malloc(sizeof(FILE));
          stream->mntpnt=mntpnt->mntpnt;
          stream->path=path;
          stream->type=mntpnt->type;
          stream->pos=0;
          stream->eof=0;
          char ok=drvs[mntpnt->type](FSOP_OPEN,stream,NULL,NULL);
          if (ok) {
            files[fd]=stream;
            char* str=malloc(sizeof(char)*11);
            int_to_ascii(fd,str);
            send_msg(sender,str);
          } else {
            free(stream);
            send_msg(sender,"4294967295");
          }
        }
        send_msg(sender,"4294967295");
      }
    }
    yield();
  }
}
