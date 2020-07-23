#define DEVFS_H
#ifndef DEVFS_H

typedef int (*dev_drv)(char* filename,int c,long pos,char wr);

void init_devfs();
void devfs_add(dev_drv drv,char* name);

#endif
