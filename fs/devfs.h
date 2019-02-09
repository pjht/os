#ifndef DEVFS_H
#define DEVFS_H

typedef int (*dev_drv)(char* filename,int c,long pos,char wr);

void init_devfs();
void add_dev(dev_drv drv,char* name);
#endif
