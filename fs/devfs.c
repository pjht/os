#include "../kernel/vfs.h"
#include "../libc/stdlib.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
#include "devfs.h"

char** devices;
dev_drv* dev_drivers;
uint32_t num_devices;
uint32_t max_devices;

char devfs_drv(fs_op op,FILE* stream,void* data1,void* data2) {
  if (op==FSOP_MOUNT) {
    return 1;
  }
  if (op==FSOP_OPEN) {
    for (int i=0;i<num_devices;i++) {
      if (strcmp(devices[i],stream->path)==0) {
        return 1;
      }
    }
    return 0;
  }
  if (op==FSOP_GETC) {
    int i;
    for (i=0;i<num_devices;i++) {
      if (strcmp(devices[i],stream->path)==0) {
        break;
      }
    }
    *((int*)data1)=dev_drivers[i]((char*)stream->path,0,stream->pos,0);
    stream->pos+=1;
    return 1;
  }
  if (op==FSOP_PUTC) {
    int i;
    for (i=0;i<num_devices;i++) {
      if (strcmp(devices[i],stream->path)==0) {
        break;
      }
    }
    dev_drivers[i]((char*)stream->path,*((int*)data1),stream->pos,1);
    stream->pos+=1;
    return 1;
  }
  if (op==FSOP_CLOSE) {
    return 1;
  }
  return 0;
}

void init_devfs() {
  devices=malloc(sizeof(char*)*32);
  dev_drivers=malloc(sizeof(dev_drv)*32);
  num_devices=0;
  max_devices=0;
  register_fs(devfs_drv,"devfs");
  mount("/dev/","","devfs");
}

void add_dev(dev_drv drv,char* name) {
  if (num_devices==max_devices) {
    devices=realloc(devices,sizeof(char*)*(max_devices+32));
    dev_drivers=realloc(dev_drivers,sizeof(dev_drv)*(max_devices+32));
    max_devices+=32;
  }
  dev_drivers[num_devices]=drv;
  devices[num_devices]=malloc(sizeof(char)*(strlen(name)+1));
  strcpy(devices[num_devices],name);
  num_devices++;
}
