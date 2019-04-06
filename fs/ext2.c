#include "../kernel/vfs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ext2_structs.h"

ext2_superblock* supblks;
uint32_t* blk_size;
blk_grp** blk_grps;
char** mnts;
char** devs;
int num_mnts;
int max_mnts;

typedef struct {
  int num;
  inode inode;
  char is_cont_valid;
  char* contents;
} file_info;

void* read_blk(int blknum,FILE* f) {
  void* block=malloc(sizeof(uint8_t)*1024);
  fseek(f,blknum*1024,SEEK_SET);
  fread(block,1,sizeof(uint8_t)*1024,f);
  return block;
}

inode read_inode(uint32_t inode_num,FILE* f,int num) {
  ext2_superblock supblk=supblks[num];
  uint32_t grp=(inode_num-1)/supblk.s_inodes_per_group;
  uint32_t index=(inode_num-1)%supblk.s_inodes_per_group;
  uint32_t starting_blk=blk_grps[num][grp].bg_inode_table;
  size_t inodes_per_blk=1024/sizeof(inode);
  uint32_t blk=starting_blk+(index/inodes_per_blk);
  uint32_t offset=index%inodes_per_blk;
  inode* inodes=read_blk(blk,f);
  return inodes[offset];
}

char** get_dir_listing(uint32_t inode_num,FILE* f,int num) {
  char** names=malloc(sizeof(char*)*100);
  int num_entries_used=0;
  int max_len=100;
  inode dir_inode=read_inode(inode_num,f,num);
  uint32_t size=dir_inode.i_size;
  uint32_t tot_size=0;
  dir_entry* dir=read_blk(dir_inode.i_block[0],f);
  dir_entry* current_entry=dir;
  for(int i=0;tot_size<size;i++) {
    if (current_entry->file_type==0) {
      break;
    }
    if(num_entries_used==max_len) {
      max_len+=100;
      names=realloc(names,sizeof(char*)*max_len);
    }
    names[num_entries_used]=malloc(current_entry->name_len+1);
    strcpy(names[num_entries_used],current_entry->file_name);
    names[num_entries_used][(int)current_entry->name_len]='\0';
    num_entries_used++;
    tot_size+=current_entry->rec_len;
    current_entry=(dir_entry*)(((uint32_t)current_entry)+current_entry->rec_len);
  }
  if(num_entries_used==max_len) {
    max_len+=1;
    names=realloc(names,sizeof(char*)*max_len);
  }
  names[num_entries_used]=NULL;
  return names;
}

void free_dir_listing(char** names) {
  for(int i=0;names[i]!=NULL;i++) {
    free(names[i]);
  }
  free(names);
}

dir_entry* read_dir_entry(uint32_t inode_num,uint32_t dir_entry_num,FILE* f,int num) {
  inode dir_inode=read_inode(inode_num,f,num);
  uint32_t size=dir_inode.i_size;
  uint32_t tot_size=0;
  uint32_t ent_num=0;
  dir_entry* dir=read_blk(dir_inode.i_block[0],f);
  dir_entry* current_entry=dir;
  for(int i=0;tot_size<size;i++) {
    if (current_entry->file_type==0) {
      break;
    }
    if(ent_num==dir_entry_num) {
      return current_entry;
    }
    ent_num++;
    tot_size+=current_entry->rec_len;
    current_entry=(dir_entry*)(((uint32_t)current_entry)+current_entry->rec_len);
  }
  return NULL;
}

uint32_t inode_for_fname(uint32_t dir_inode_num, char* name, char* got_inode,FILE* f,int num) {
  uint32_t inode=0;
  *got_inode=0;
  char** names=get_dir_listing(dir_inode_num,f,num);
  for(int i=0;names[i]!=NULL;i++) {
    if (strcmp(names[i],name)==0) {
      dir_entry* entry=read_dir_entry(dir_inode_num,i,f,num);
      inode=entry->inode;
      *got_inode=1;
      break;
    }
  }
  free_dir_listing(names);
  return inode;
}

char* fname_for_inode(uint32_t dir_inode_num, uint32_t inode_num,FILE* f,int num) {
  for(int i=0;;i++) {
    dir_entry* entry=read_dir_entry(dir_inode_num,i,f,num);
    if (entry) {
      if (entry->inode==inode_num) {
        char* name=malloc(strlen(entry->file_name)+1);
        strcpy(name,entry->file_name);
        return name;
      }
    } else {
      break;
    }
  }
  return NULL;
}

static char drv(fs_op op,FILE* stream,void* data1,void* data2) {
  if (op==FSOP_MOUNT) {
    char* dev=(char*)data2;
    if (max_mnts==num_mnts) {
        supblks=realloc(supblks,sizeof(ext2_superblock)*(max_mnts+32));
        blk_size=realloc(blk_size,sizeof(uint32_t)*(max_mnts+32));
        blk_grps=realloc(blk_grps,sizeof(blk_grp*)*(max_mnts+32));
        mnts=realloc(mnts,sizeof(char*)*(max_mnts+32));
        devs=realloc(devs,sizeof(char*)*(max_mnts+32));
        max_mnts+=32;
    }
    FILE* f=fopen(dev,"r");
    ext2_superblock* supblk=read_blk(1,f);
    supblks[num_mnts]=*supblk;
    blk_size[num_mnts]=1024<<(supblk->s_log_blk_size);
    double num_blk_grps_dbl=supblk->s_blocks_count/(double)supblk->s_blocks_per_group;
    uint32_t num_blk_grps=ceil(num_blk_grps_dbl);
    blk_grps=malloc(sizeof(blk_grp*)*num_blk_grps);
    blk_grp* blk_group=read_blk(2,f);
    blk_grps[num_mnts]=malloc(sizeof(blk_grp)*num_blk_grps);
    for (uint32_t i=0;i<num_blk_grps;i++) {
      blk_grps[num_mnts][i]=*blk_group;
      blk_group++;
    };
    fclose(f);
    mnts[num_mnts]=(char*)data1;
    devs[num_mnts]=dev;
    num_mnts++;
    return 1;
  }
  if (op==FSOP_OPEN) {
    file_info* data=NULL;
    for (int i=0;i<num_mnts;i++) {
      if (strcmp(mnts[i],stream->mntpnt)==0) {
        stream->data=malloc(sizeof(file_info));
        data=stream->data;
        data->num=i;
        data->is_cont_valid=0;
        break;
      }
    }
    if (data) {
      FILE* f=fopen(devs[data->num],"r");
      char got_inode;
      uint32_t inode_num=inode_for_fname(2,stream->path,&got_inode,f,data->num);
      if (got_inode) {
        data->inode=read_inode(inode_num,f,data->num);
        fclose(f);
        return 1;
      } else {
        fclose(f);
        return 0;
      }
    } else {
      return 0;
    }
  }
  if (op==FSOP_GETC) {
    file_info* data=stream->data;
    char* contents;
    if (data->is_cont_valid) {
        contents=data->contents;
    } else {
      FILE* f=fopen(devs[data->num],"r");
      contents=read_blk((data->inode).i_block[0],f);
      fclose(f);
      data->is_cont_valid=1;
      data->contents=contents;
    }
    if (stream->pos>strlen(contents)) {
      *((int*)data1)=EOF;
      stream->eof=1;
      return 1;
    }
    *((int*)data1)=contents[stream->pos];
    stream->pos++;
    return 1;
  }
  if (op==FSOP_PUTC) {
    return 1;
  }
  if (op==FSOP_CLOSE) {
    return 1;
  }
  return 0;
}

void init_ext2() {
  supblks=malloc(sizeof(ext2_superblock)*32);
  blk_size=malloc(sizeof(uint32_t)*32);
  blk_grps=malloc(sizeof(blk_grp*)*32);
  mnts=malloc(sizeof(char*)*32);
  devs=malloc(sizeof(char*)*32);
  num_mnts=0;
  max_mnts=32;
  register_fs(drv,"ext2");
}
