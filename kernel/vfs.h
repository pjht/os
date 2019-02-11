#ifndef VFS_H
#define VFS_H
#include <stdint.h>
#include "../libc/stdio.h"

typedef enum {
  FSOP_MOUNT,
  FSOP_OPEN,
  FSOP_GETC,
  FSOP_PUTC,
  FSOP_CLOSE,
  FSOP_UMOUNT
} fs_op;

#define NO_FD 0xFFFFFFFF

extern char vfs_initialized;

typedef char (*fs_drv)(fs_op op,FILE* stream,void* data1,void* data2);

uint32_t register_fs(fs_drv drv,const char* type);
char mount(char* mntpnt,char* dev,char* type);
void vfs_task();

#endif
