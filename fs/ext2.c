#include "../kernel/vfs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
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

void* read_blk(int blknum,FILE* f,int num) {
  void* block=malloc(sizeof(uint8_t)*blk_size[num]);
  fseek(f,blknum*blk_size[num],SEEK_SET);
  fread(block,1,sizeof(uint8_t)*blk_size[num],f);
  return block;
}

inode read_inode(uint32_t inode_num,FILE* f,int num) {
  ext2_superblock supblk=supblks[num];
  uint32_t grp=(inode_num-1)/supblk.s_inodes_per_group;
  uint32_t index=(inode_num-1)%supblk.s_inodes_per_group;
  uint32_t starting_blk=blk_grps[num][grp].bg_inode_table;
  size_t inodes_per_blk=blk_size[num]/sizeof(inode);
  uint32_t blk=starting_blk+(index/inodes_per_blk);
  uint32_t offset=index%inodes_per_blk;
  inode* inodes=read_blk(blk,f,num);
  return inodes[offset];
}

uint64_t get_sz(inode inode,int num) {
  uint64_t size=inode.i_size;
  if (supblks[num].s_feature_rw_compat&EXT2_FEATURE_RW_COMPAT_LARGE_FILE) {
    size=size|(((uint64_t)inode.i_ext_size_or_dir_acl)<<32);
  }
  return size;
}

int read_char(inode inode,FILE* f,int pos,int num) {
  if (inode.i_block[0]==0) {
    return -1;
  }
  uint64_t size=get_sz(inode,num);
  int block=pos/blk_size[num];
  pos=pos%blk_size[num];
  if (block<12) {
    if (inode.i_block[block]==0) {
      return -1;
    } else {
      return ((char*)read_blk(inode.i_block[block],f,num))[pos];
    }
  } else if (block<268) {
    uint32_t* blocks=read_blk(inode.i_block[12],f,num);
    if (blocks[block-12]==0) {
      return -1;
    } else {
      return ((char*)read_blk(blocks[block],f,num))[pos];
    }
  }
  return -1;
}

void* read_inode_contents(inode inode,FILE* f,int num) {
  if (inode.i_block[0]==0) {
    return NULL;
  }
  uint64_t size=get_sz(inode,num);
  char* data=malloc(sizeof(char)*ceil(size/blk_size[num]));
  for (int i=0;i<12;i++) {
    if (inode.i_block[i]==0) {
      break;
    }
    memcpy(&data[i*blk_size[num]],read_blk(inode.i_block[i],f,num),blk_size[num]);
  }
  if (inode.i_block[12]!=0) {
    uint32_t* blocks=read_blk(inode.i_block[12],f,num);
    for (int i=0;i<256;i++) {
      if (blocks[i]==0) {
        break;
      }
      memcpy(&data[(i+12)*blk_size[num]],read_blk(blocks[i],f,num),blk_size[num]);
    }
  }
  return (void*)data;
}

char** get_dir_listing(uint32_t inode_num,FILE* f,int num) {
  char** names=malloc(sizeof(char*)*100);
  int num_entries_used=0;
  int max_len=100;
  inode dir_inode=read_inode(inode_num,f,num);
  uint32_t size=dir_inode.i_size;
  uint32_t tot_size=0;
  dir_entry* dir=read_inode_contents(dir_inode,f,num);
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
    names[num_entries_used]=current_entry->file_name;
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
  dir_entry* dir=read_inode_contents(dir_inode,f,num);
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
  //free_dir_listing(names);
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
    fseek(f,1024,SEEK_SET);
    ext2_superblock* supblk=(ext2_superblock*)malloc(sizeof(char)*1024);
    fread(supblk,1024,1,f);
    supblks[num_mnts]=*supblk;
    blk_size[num_mnts]=1024<<(supblk->s_log_blk_size);
    double num_blk_grps_dbl=supblk->s_blocks_count/(double)supblk->s_blocks_per_group;
    uint32_t num_blk_grps=ceil(num_blk_grps_dbl);
    blk_grp* blk_group=read_blk(2,f,num_mnts);
    uint32_t num_blks=((sizeof(blk_grp)*num_blk_grps)/blk_size[num_mnts])+1;
    blk_grps[num_mnts]=malloc(sizeof(uint8_t)*num_blks*1024);
    char* data_ptr=(char*)(blk_grps[num_mnts]);
    for (int i=0;i<num_blks;i++) {
      memcpy(&data_ptr[i*blk_size[num_mnts]],read_blk(i+2,f,num_mnts),blk_size[num_mnts]);
    }
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
      uint32_t inode_num=2;
      inode inode;
      for (char* tok=strtok(stream->path,"/");tok!=NULL;tok=strtok(NULL,"/")) {
        char got_inode;
        inode_num=inode_for_fname(inode_num,tok,&got_inode,f,data->num);
        if (got_inode) {
          inode=read_inode(inode_num,f,data->num);
          if ((inode.i_mode&EXT2_S_IFDIR)==0) {
            char* next_tok=strtok(NULL,"/");
            if (next_tok) {
              klog("INFO","%s: Not a directory",tok);
              fclose(f);
              return 0;
            } else {
              break;
            }
          }
        } else {
          klog("INFO","%s: No such file or directory",tok);
          fclose(f);
          return 0;
        }
      }
      data->inode=inode;
      fclose(f);
      return 1;
    } else {
      return 0;
    }
  }
  if (op==FSOP_GETC) {
    file_info* data=stream->data;
    //char* contents;
    // if (data->is_cont_valid) {
    //     contents=data->contents;
    // } else {
    //   FILE* f=fopen(devs[data->num],"r");
    //   contents=read_inode_contents(data->inode,f,data->num);
    //   fclose(f);
    //   data->is_cont_valid=1;
    //   data->contents=contents;
    // }
    if (stream->pos>get_sz(data->inode,data->num)) {
      *((int*)data1)=EOF;
      stream->eof=1;
      return 1;
    }
    FILE* f=fopen(devs[data->num],"r");
    *((int*)data1)=read_char(data->inode,f,stream->pos,data->num);
    fclose(f);
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
