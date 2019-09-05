#include <mailboxes.h>
#include <string.h>
#include <stdlib.h>
#include <ipc/vfs.h>
#include <stdio.h>
#include <tasking.h>
#include <dbg.h>
#include <limits.h>
#include <unistd.h>
#define VFS_PID 2


static uint32_t box;
static uint32_t vfs_box;
FILE* __stdio_stdin;
FILE* __stdio_stdout;
FILE* __stdio_stderr;


void __stdio_init() {
  char name[256];
  strcpy(name,"stdio");
  int name_end_index=strlen(name);
  char* name_end=&name[name_end_index];
  int_to_ascii(getpid(),name_end);
  box=mailbox_new(16,name);
  __stdio_stdin=malloc(sizeof(FILE*));
  *__stdio_stdin=0;
  __stdio_stdout=malloc(sizeof(FILE*));
  *__stdio_stdout=1;
  __stdio_stderr=malloc(sizeof(FILE*));
  *__stdio_stderr=2;
  vfs_box=mailbox_find_by_name("vfs");
  if (vfs_box==0) {
    serial_print("Cannot find VFS box\n");
  }
}

static vfs_message* make_msg(vfs_message_type type,const char* mode,const char* path, uint32_t fd, int data) {
  vfs_message* msg_data=malloc(sizeof(vfs_message));
  msg_data->type=type;
  msg_data->id=0;
  msg_data->fd=fd;
  msg_data->data=data;
  msg_data->in_progress=0;
  msg_data->orig_mbox=box;
  if (mode!=NULL) {
    strcpy(&msg_data->mode[0],mode);
  }
  if (path!=NULL) {
    strcpy(&msg_data->path[0],path);
  }
  return msg_data;
}

FILE* fopen(char* filename,char* mode) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return NULL;
  }
  if (strlen(filename)>4096 || strlen(mode)>10) {
    return NULL;
  }
  vfs_message* msg_data=make_msg(VFS_OPEN,mode,filename,0,0);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return NULL;
  } else {
    FILE* file=malloc(sizeof(FILE));
    *file=vfs_msg->fd; //We're using pointers to FILE even though it's a uint32_t so we can expand to a struct if needed
    free(msg.msg);
    return file;
  }
}

int putc(int c, FILE* stream) __attribute__ ((alias ("fputc")));

int fputc(int c, FILE* stream) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return EOF;
  }
  char str[]={c,'\0'};
  if (fputs(str,stream)==0) {
    return c;
  } else {
    return EOF;
  }
  return EOF;
}

int getc(FILE* stream) __attribute__ ((alias ("fgetc")));

int fgetc(FILE* stream) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return EOF;
  }
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
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return NULL;
  }
  vfs_message* msg_data=make_msg(VFS_GETS,NULL,NULL,*stream,count);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(vfs_msg);
    return NULL;
  } else {
    msg.msg=str;
    mailbox_get_msg(box,&msg,count);
    while (msg.from==0) {
      yieldToPID(VFS_PID);
      mailbox_get_msg(box,&msg,count);
    }
    str[vfs_msg->data]='\0';
    free(vfs_msg);
    return str;
  }
  free(vfs_msg);
  return NULL;
}

size_t fread(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return 0;
  }
  char* buffer=(char*)buffer_ptr;
  size_t bytes=size*count;
  vfs_message* msg_data=make_msg(VFS_GETS,NULL,NULL,*stream,bytes);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(vfs_msg);
    return 0;
  } else {
    msg.msg=buffer;
    mailbox_get_msg(box,&msg,count);
    while (msg.from==0) {
      yieldToPID(VFS_PID);
      mailbox_get_msg(box,&msg,count);
    }
    free(vfs_msg);
    return count;
  }
  free(vfs_msg);
  return 0;
}

int fputs(const char* s, FILE* stream) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return EOF;
  }
  vfs_message* msg_data=make_msg(VFS_PUTS,NULL,NULL,*stream,strlen(s));
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=(char*)s;
  msg.size=strlen(s);
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return EOF;
  } else {
    free(msg.msg);
    return 0;
  }
  free(msg.msg);
  return EOF;
}

size_t fwrite(void* buffer_ptr,size_t size,size_t count,FILE* stream) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return 0;
  }
  char* buffer=(char*)buffer_ptr;
  size_t bytes=size*count;
  vfs_message* msg_data=make_msg(VFS_PUTS,NULL,NULL,*stream,bytes);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=buffer;
  msg.size=bytes;
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  vfs_message* vfs_msg=(vfs_message*)msg.msg;
  if (vfs_msg->flags) {
    free(msg.msg);
    return 0;
  } else {
    free(msg.msg);
    return count;
  }
  free(msg.msg);
  return 0;
}

void register_fs(const char* name,uint32_t mbox) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return;
  }
  vfs_message* msg_data=make_msg(VFS_REGISTER_FS,name,NULL,mbox,0);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yieldToPID(VFS_PID);
  msg.msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  free(msg.msg);
}

void mount(char* file,char* type,char* path) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return;
  }
  vfs_message* msg_data=make_msg(VFS_MOUNT,type,path,0,strlen(file)+1);
  Message msg;
  msg.from=box;
  msg.to=vfs_box;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  msg.msg=file;
  msg.size=strlen(file)+1;
  mailbox_send_msg(&msg);
  yieldToPID(VFS_PID);
  msg.msg=msg_data;
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  while (msg.from==0) {
    yieldToPID(VFS_PID);
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
  }
  free(msg.msg);
}

int vfprintf(FILE* stream,const char* format,va_list arg) {
  if (vfs_box==0) {
    serial_print("The VFS box has not been found\n");
    return EOF;
  }
  int c;
	for(;*format!='\0';format++) {
    if(*format!='%') {
  		c=fputc(*format,stream);
      if (c==EOF) {
        return EOF;
      }
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
        uint32_t i=va_arg(arg,uint32_t);
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

void rescan_vfs() {
  vfs_box=mailbox_find_by_name("vfs");
}
