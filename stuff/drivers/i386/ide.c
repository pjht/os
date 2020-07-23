#include "../../cpu/i386/ports.h"
#include "../../fs/devfs.h"
#include <klog.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define IDE_PRIM_IO 0x1f0
#define IDE_PRIM_CTRL 0x3f6
#define IDE_SEC_IO 0x170
#define IDE_SEC_CTRL 0x376

static uint8_t ident[4][512];
static uint8_t* sect_data=NULL;
static uint32_t last_read_sector=0;
static int last_read_base=0;
static int last_read_slave=0;
static uint8_t* read_sect(int base,int slave,uint32_t lba) {
  if (last_read_sector==lba && last_read_base==base && last_read_slave==slave && sect_data) {
    return sect_data;
  }
  port_byte_out(base+6,0xe0|slave<<4|((lba&0xFF000000)>>24));
  for (int i=0;i<4;i++) port_byte_in(base+7);
  while ((port_byte_in(base+7)&0x80)!=0);
  port_byte_out(base+2,1);
  port_byte_out(base+3,lba&0xFF);
  port_byte_out(base+4,(lba&0xFF00)>>8);
  port_byte_out(base+5,(lba&0xFF0000)>>16);
  port_byte_out(base+7,0x20);
  uint8_t* sect=malloc(sizeof(uint8_t)*512);
  while ((port_byte_in(base+7)&0x88)!=0x8);
  for (int i=0;i<512;i+=2) {
    uint16_t data=port_word_in(base);
    sect[i]=data&0xFF;
    sect[i+1]=(data&0xFF00)>>8;
  }
  last_read_sector=lba;
  last_read_base=base;
  last_read_slave=slave;
  // if (sect_data) {
  //   free(sect_data);
  // }
  sect_data=sect;
  return sect;
}

static void write_sect(int base,int slave,uint32_t lba,uint8_t* sect) {
  if (last_read_sector==lba && last_read_base==base && last_read_slave==slave && sect_data) {
    sect_data=sect;
  }
  if (!sect_data) {
    last_read_sector=lba;
    last_read_base=base;
    last_read_slave=slave;
    sect_data=sect;
  }
  port_byte_out(base+6,0xe0|slave<<4|((lba&0xFF000000)>>24));
  for (int i=0;i<4;i++) port_byte_in(base+7);
  while ((port_byte_in(base+7)&0x80)!=0);
  port_byte_out(base+2,1);
  port_byte_out(base+3,lba&0xFF);
  port_byte_out(base+4,(lba&0xFF00)>>8);
  port_byte_out(base+5,(lba&0xFF0000)>>16);
  port_byte_out(base+7,0x30);
  while ((port_byte_in(base+7)&0x88)!=0x8);
  for (int i=0;i<512;i+=2) {
    port_word_out(base,sect[i]|(sect[i+1]<<8));
    for (int i=0;i<4;i++) port_byte_in(base+7);
  }
  while ((port_byte_in(base+7)&0x80)!=0);
  port_byte_out(base+7,0xE7);
}

static int drv(char* filename,int c,long pos,char wr) {
  int base;
  int slave;
  if (strcmp(filename,"hda")==0) {
    base=IDE_PRIM_IO;
    slave=0;
  }
  if (strcmp(filename,"hdb")==0) {
    base=IDE_PRIM_IO;
    slave=1;
  }
  if (strcmp(filename,"hdc")==0) {
    base=IDE_SEC_IO;
    slave=0;
  }
  if (strcmp(filename,"hdc")==0) {
    base=IDE_SEC_IO;
    slave=1;
  }
  if (wr) {
    uint32_t lba=pos/512;
    int offset=pos%512;
    uint8_t* sect=read_sect(base,slave,lba);
    sect[offset]=(uint8_t)c;
    write_sect(base,slave,lba,sect);
    return 1;
  } else {
    uint32_t lba=pos/512;
    int offset=pos%512;
    uint8_t* sect=read_sect(base,slave,lba);
    uint8_t val=sect[offset];
    return val;
  }
}

void ide_init() {
  if (port_byte_in(IDE_PRIM_IO+7)!=0xFF) {
    //Detect primary master
    port_byte_out(IDE_PRIM_IO+6,0xe0);
    for (int i=0;i<4;i++) port_byte_in(IDE_PRIM_IO+7);
    while ((port_byte_in(IDE_PRIM_IO+7)&0x80)!=0);
    port_byte_out(IDE_PRIM_IO+2,0);
    port_byte_out(IDE_PRIM_IO+3,0);
    port_byte_out(IDE_PRIM_IO+4,0);
    port_byte_out(IDE_PRIM_IO+5,0);
    port_byte_out(IDE_PRIM_IO+7,0xEC);
    if (port_byte_in(IDE_PRIM_IO+7)!=0) {
      uint8_t io4=port_byte_in(IDE_PRIM_IO+4);
      uint8_t io5=port_byte_in(IDE_PRIM_IO+5);
      if (io4==0x14&&io5==0xeb) {
        klog("INFO","IDE primary master is ATAPI");
      }
      if (io4==0x3c&&io5==0xc3) {
        klog("INFO","IDE primary master is SATA");
      }
      if (io4==0&&io5==0) {
        while ((port_byte_in(IDE_PRIM_IO+7)&0x8)!=0x8);
        for (int i=0;i<512;i+=2) {
          uint16_t data=port_word_in(IDE_PRIM_IO);
          ident[0][i]=data&0xFF;
          ident[0][i+1]=(data&0xFF00)>>8;
        }
        klog("INFO","Found IDE primary master");
        port_byte_out(IDE_PRIM_CTRL,port_byte_in(IDE_PRIM_CTRL)|0x2);
        devfs_add(drv,"hda");
      }
    }
    //Detect primary slave
    port_byte_out(IDE_PRIM_IO+6,0xf0);
    for (int i=0;i<4;i++) port_byte_in(IDE_PRIM_IO+7);
    while ((port_byte_in(IDE_PRIM_IO+7)&0x80)!=0);
    port_byte_out(IDE_PRIM_IO+2,0);
    port_byte_out(IDE_PRIM_IO+3,0);
    port_byte_out(IDE_PRIM_IO+4,0);
    port_byte_out(IDE_PRIM_IO+5,0);
    port_byte_out(IDE_PRIM_IO+7,0xEC);
    if (port_byte_in(IDE_PRIM_IO+7)!=0) {
      uint8_t io4=port_byte_in(IDE_PRIM_IO+4);
      uint8_t io5=port_byte_in(IDE_PRIM_IO+5);
      if (io4==0x14&&io5==0xeb) {
        klog("INFO","IDE primary slave is ATAPI");
      }
      if (io4==0x3c&&io5==0xc3) {
        klog("INFO","IDE primary slave is SATA");
      }
      if (io4==0&&io5==0) {
        while ((port_byte_in(IDE_PRIM_IO+7)&0x8)!=0x8);
        for (int i=0;i<512;i+=2) {
          uint16_t data=port_word_in(IDE_PRIM_IO);
          ident[0][i]=data&0xFF;
          ident[0][i+1]=(data&0xFF00)>>8;
        }
        klog("INFO","Found IDE primary slave");
        port_byte_out(IDE_PRIM_CTRL,port_byte_in(0x3f6)|0x2);
        devfs_add(drv,"hdb");
      }
    }
  }
  // if (port_byte_in(IDE_SEC_IO+7)!=0xFF) {
  //   //Detect secondary master
  //   port_byte_out(IDE_SEC_IO+6,0xe0);
  //   for (int i=0;i<4;i++) port_byte_in(IDE_SEC_IO+7);
  //   while ((port_byte_in(IDE_SEC_IO+7)&0x80)!=0);
  //   port_byte_out(IDE_SEC_IO+2,0);
  //   port_byte_out(IDE_SEC_IO+3,0);
  //   port_byte_out(IDE_SEC_IO+4,0);
  //   port_byte_out(IDE_SEC_IO+5,0);
  //   port_byte_out(IDE_SEC_IO+7,0xEC);
  //   if (port_byte_in(IDE_SEC_IO+7)!=0) {
  //     if (port_byte_in(IDE_SEC_IO+5)!=0&&port_byte_in(IDE_SEC_IO+6)!=0) {
  //     }
  //     while ((port_byte_in(IDE_SEC_IO+7)&0x8)!=0x8);
  //     for (int i=0;i<512;i+=2) {
  //       uint16_t data=port_word_in(IDE_SEC_IO);
  //       ident[0][i]=data&0xFF;
  //       ident[0][i+1]=(data&0xFF00)>>8;
  //     }
  //     klog("INFO","Found IDE secondary master");
  //     port_byte_out(IDE_SEC_CTRL,port_byte_in(IDE_SEC_CTRL)|0x2);
  //     devfs_add(drv,"hdc");
  //   }
  //   //Detect secondary slave
  //   port_byte_out(IDE_SEC_IO+6,0xf0);
  //   for (int i=0;i<4;i++) port_byte_in(IDE_SEC_IO+7);
  //   while ((port_byte_in(IDE_SEC_IO+7)&0x80)!=0);
  //   port_byte_out(IDE_SEC_IO+2,0);
  //   port_byte_out(IDE_SEC_IO+3,0);
  //   port_byte_out(IDE_SEC_IO+4,0);
  //   port_byte_out(IDE_SEC_IO+5,0);
  //   port_byte_out(IDE_SEC_IO+7,0xEC);
  //   if (port_byte_in(IDE_SEC_IO+7)!=0) {
  //     if (port_byte_in(IDE_SEC_IO+5)!=0&&port_byte_in(IDE_SEC_IO+6)!=0) {
  //     }
  //     while ((port_byte_in(IDE_SEC_IO+7)&0x8)!=0x8);
  //     for (int i=0;i<512;i+=2) {
  //       uint16_t data=port_word_in(IDE_SEC_IO);
  //       ident[0][i]=data&0xFF;
  //       ident[0][i+1]=(data&0xFF00)>>8;
  //     }
  //     klog("INFO","Found IDE decondaryary slave");
  //     port_byte_out(IDE_SEC_CTRL,port_byte_in(IDE_SEC_CTRL)|0x2);
  //     devfs_add(drv,"hdd");
  //   }
  // }
}
