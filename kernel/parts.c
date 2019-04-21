#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <klog.h>
#include "../fs/devfs.h"

typedef struct {
  char bootable;
  char chs_start[3];
  char fs_id;
  char chs_end[3];
  uint32_t start_lba;
  uint32_t length;
} partition;

static const char** part_devs=NULL;
static partition** parts=NULL;
static uint32_t num_part_devs=0;
static uint32_t max_part_devs=0;

int drv(char* filename,int c,long pos,char wr) {
  int part_no;
  switch (filename[strlen(filename)-1]) {
    case '1':
      part_no=0;
      break;
    case '2':
      part_no=1;
      break;
    case '3':
      part_no=2;
      break;
    case '4':
      part_no=3;
      break;
  }
  char* str=malloc(sizeof(char)*(strlen(filename)+1));
  strcpy(str,filename);
  str[strlen(str)-1]='\0';
  uint32_t i;
  for (i=0;i<num_part_devs;i++) {
    if (strcmp(part_devs[i]+5,str)==0) {
      break;
    }
  }
  free(str);
  uint32_t lba=pos/512;
  int offset=pos%512;
  if (lba>parts[i][part_no].length) {
    klog("INFO","Outside partition boundary");
    for(;;);
    return 0;
  }
  lba+=parts[i][part_no].start_lba;
  pos=lba*512;
  pos+=offset;
  if (wr) {
    FILE* f=fopen(part_devs[i],"w");
    fseek(f,pos,SEEK_SET);
    fputc(c,f);
    fclose(f);
    return 1;
  } else {
    FILE* f=fopen(part_devs[i],"r");
    fseek(f,pos,SEEK_SET);
    int c=fgetc(f);
    fclose(f);
    return c;
  }
  return 0;
}


void load_parts(const char* path) {
  if (num_part_devs==max_part_devs) {
    max_part_devs+=32;
    part_devs=realloc(part_devs,sizeof(char*)*max_part_devs);
    parts=realloc(parts,sizeof(partition*)*num_part_devs);
  }
  FILE* f=fopen(path,"r");
  part_devs[num_part_devs]=path;
  path+=5;
  parts[num_part_devs]=malloc(sizeof(partition)*4);
  fseek(f,0x1BE,SEEK_SET);
  fread(parts[num_part_devs],sizeof(partition),4,f);
  for (int i=0;i<4;i++) {
    if (parts[num_part_devs][i].fs_id!=0) {
      klog("INFO","Found partition %d of type %x on sectors %d-%d ",i,(uint8_t)parts[num_part_devs][i].fs_id,parts[num_part_devs][i].start_lba,parts[num_part_devs][i].start_lba+parts[num_part_devs][i].length);
      char str[2];
      int_to_ascii(i+1,str);
      char* part_path=malloc(sizeof(char)*strlen(path)+2);
      memcpy(part_path,path,strlen(path));
      memcpy(part_path+strlen(path),str,2);
      klog("INFO","Path:%s",part_path);
      devfs_add(drv,part_path);
    }
  }
  fclose(f);
  num_part_devs++;
}
