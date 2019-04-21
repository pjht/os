#include "../kernel/vfs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <klog.h>
#include "ext2_structs.h"

ext2_superblock* supblks;
uint32_t* blk_size;
blk_grp** blk_grps;
uint32_t* blk_grp_num;
char** mnts;
char** devs;
int num_mnts;
int max_mnts;

typedef struct {
  int num;
  inode* inode;
  char is_cont_valid;
  char* contents;
} file_info;

static char get_bmap_bit(uint32_t index,uint8_t* bitmap) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  char entry=bitmap[byte];
  return (entry&(1<<bit))>0;
}

static void set_bmap_bit(uint32_t index,uint8_t* bitmap) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  bitmap[byte]=bitmap[byte]|(1<<bit);
}

void* read_blk(int blknum,FILE* f,int num) {
  void* block=malloc(sizeof(uint8_t)*blk_size[num]);
  fseek(f,blknum*blk_size[num],SEEK_SET);
  fread(block,1,sizeof(uint8_t)*blk_size[num],f);
  return block;
}

void write_blk(int blknum,void* block,FILE* f,int num) {
  fseek(f,blknum*blk_size[num],SEEK_SET);
  fwrite(block,1,sizeof(uint8_t)*blk_size[num],f);
}

inode* read_inode(uint32_t inode_num,FILE* f,int num) {
  ext2_superblock supblk=supblks[num];
  uint32_t grp=(inode_num-1)/supblk.s_inodes_per_group;
  uint32_t index=(inode_num-1)%supblk.s_inodes_per_group;
  uint32_t starting_blk=blk_grps[num][grp].bg_inode_table;
  size_t inodes_per_blk=blk_size[num]/sizeof(inode);
  uint32_t blk=starting_blk+(index/inodes_per_blk);
  uint32_t offset=index%inodes_per_blk;
  inode* inodes=read_blk(blk,f,num);
  return &inodes[offset];
}

void write_inode(uint32_t inode_num,inode* node,FILE* f,int num) {
  ext2_superblock supblk=supblks[num];
  uint32_t grp=(inode_num-1)/supblk.s_inodes_per_group;
  uint32_t index=(inode_num-1)%supblk.s_inodes_per_group;
  uint32_t starting_blk=blk_grps[num][grp].bg_inode_table;
  size_t inodes_per_blk=blk_size[num]/sizeof(inode);
  uint32_t blk=starting_blk+(index/inodes_per_blk);
  uint32_t offset=index%inodes_per_blk;
  inode* inodes=read_blk(blk,f,num);
  inodes[offset]=*node;
  write_blk(blk,inodes,f,num);
}

uint32_t reserve_inode(FILE* f,int num) {
  blk_grp* grps=blk_grps[num];
  for (uint32_t i=0;i<blk_grp_num[num];i++) {
    if (grps[i].bg_free_inodes_count==0) {
      continue;
    }
    uint32_t starting_blk=grps[i].bg_inode_bitmap;
    uint32_t num_blks=((supblks[num].s_inodes_per_group/8)/blk_size[num])+1;
    uint8_t* bitmap=malloc(sizeof(uint8_t)*num_blks*blk_size[num]);
    for (uint32_t i=0;i<num_blks;i++) {
      memcpy(&bitmap[i*blk_size[num]],read_blk(i+starting_blk,f,num),blk_size[num]);
    }
    uint32_t j;
    for (j=0;j<supblks[num].s_inodes_per_group;j++) {
      if (get_bmap_bit(j,bitmap)==0) break;

    }
    set_bmap_bit(j,bitmap);
    if (get_bmap_bit(j,bitmap)==0) {
      klog("INFO","Could not reserve inode!");
      for (;;);
    } else {
      klog("INFO","Inode reserved");
    }
    for (uint32_t i=0;i<num_blks;i++) {
      write_blk(i+starting_blk,&bitmap[i*blk_size[num]],f,num);
    }
    return (256*i)+j;
  }
  return 0;
}

uint64_t get_sz(inode* node,int num) {
  uint64_t size=node->i_size;
  if (supblks[num].s_feature_rw_compat&EXT2_FEATURE_RW_COMPAT_LARGE_FILE) {
    size=size|(((uint64_t)node->i_ext_size_or_dir_acl)<<32);
  }
  return size;
}

void set_sz(inode* node,uint64_t sz,int num) {
  if (supblks[num].s_feature_rw_compat&EXT2_FEATURE_RW_COMPAT_LARGE_FILE) {
    node->i_size=(uint32_t)(sz&0xFFFFFFFF);
    node->i_ext_size_or_dir_acl=(uint32_t)((sz&0xFFFFFFFF00000000)>>32);
  } else {
    node->i_size=(uint32_t)sz;
  }
}

void inc_sz(inode* node,int num) {
  if (supblks[num].s_feature_rw_compat&EXT2_FEATURE_RW_COMPAT_LARGE_FILE) {
    node->i_size+=1;
    if (node->i_size==0) {
      node->i_ext_size_or_dir_acl+=1;
    }
  } else {
    node->i_size+=1;
  }
}
int read_char(inode* node,FILE* f,int pos,int num) {
  if (node->i_block[0]==0) {
    return -1;
  }
  int block=pos/blk_size[num];
  pos=pos%blk_size[num];
  if (block<12) {
    if (node->i_block[block]==0) {
      return -1;
    } else {
      return ((char*)read_blk(node->i_block[block],f,num))[pos];
    }
  } else if (block<268) {
    uint32_t* blocks=read_blk(node->i_block[12],f,num);
    if (blocks[block-12]==0) {
      return -1;
    } else {
      return ((char*)read_blk(blocks[block],f,num))[pos];
    }
  }
  return -1;
}

void append_char(inode* node,uint32_t inode_num,uint8_t c,FILE* f,int num) {
  uint64_t size=get_sz(node,num);
  uint32_t blk_idx=size/blk_size[num];
  uint32_t offset=size%blk_size[num];
  if (blk_idx>12||(node->i_block[blk_idx]==0)) return;
  uint32_t blk=node->i_block[blk_idx];
  uint8_t* block=read_blk(blk,f,num);
  block[offset]=c;
  write_blk(blk,block,f,num);
  inc_sz(node,num);
  write_inode(inode_num,node,f,num);
}

int write_char(inode* node,uint8_t c,uint64_t pos,FILE* f,int num) {
  uint64_t size=get_sz(node,num);
  uint32_t blk_idx=pos/blk_size[num];
  if (pos>size) {
    if (blk_idx>12) {
      return EFBIG;
    } else {
      if (node->i_block[blk_idx]==0) {
          for (uint32_t i=0;i<=blk_idx;i++) {
            // node.i_block[blk_idx]=reserve_block(f,num);
            return 0;
          }
      } else {
        set_sz(node,size++,num);
      }
    }
  }
  uint32_t offset=pos%blk_size[num];
  if (blk_idx>12||(node->i_block[blk_idx]==0)) return 0;
  uint32_t blk=node->i_block[blk_idx];
  uint8_t* block=read_blk(blk,f,num);
  block[offset]=c;
  write_blk(blk,block,f,num);
  return 0;
}

void* read_inode_contents(inode* node,FILE* f,int num) {
  if (node->i_block[0]==0) {
    return NULL;
  }
  uint64_t size=get_sz(node,num);
  char* data=malloc(sizeof(char)*ceil(size/blk_size[num]));
  for (int i=0;i<12;i++) {
    if (node->i_block[i]==0) {
      break;
    }
    memcpy(&data[i*blk_size[num]],read_blk(node->i_block[i],f,num),blk_size[num]);
  }
  if (node->i_block[12]!=0) {
    uint32_t* blocks=read_blk(node->i_block[12],f,num);
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
  inode* dir_inode=read_inode(inode_num,f,num);
  uint32_t size=dir_inode->i_size;
  uint32_t tot_size=0;
  ext2_dir_entry* dir=read_inode_contents(dir_inode,f,num);
  ext2_dir_entry* current_entry=dir;
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
    current_entry=(ext2_dir_entry*)(((uint32_t)current_entry)+current_entry->rec_len);
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

ext2_dir_entry* read_dir_entry(uint32_t inode_num,uint32_t dir_entry_num,FILE* f,int num) {
  inode* dir_inode=read_inode(inode_num,f,num);
  uint32_t size=dir_inode->i_size;
  uint32_t tot_size=0;
  uint32_t ent_num=0;
  ext2_dir_entry* dir=read_inode_contents(dir_inode,f,num);
  ext2_dir_entry* current_entry=dir;
  for(int i=0;tot_size<size;i++) {
    if (current_entry->file_type==0) {
      break;
    }
    if(ent_num==dir_entry_num) {
      return current_entry;
    }
    ent_num++;
    tot_size+=current_entry->rec_len;
    current_entry=(ext2_dir_entry*)(((uint32_t)current_entry)+current_entry->rec_len);
  }
  return NULL;
}

uint32_t inode_for_fname(uint32_t dir_inode_num, char* name, char* got_inode,FILE* f,int num) {
  uint32_t node=0;
  *got_inode=0;
  char** names=get_dir_listing(dir_inode_num,f,num);
  for(int i=0;names[i]!=NULL;i++) {
    if (strcmp(names[i],name)==0) {
      dir_entry* entry=read_dir_entry(dir_inode_num,i,f,num);
      node=entry->inode;
      *got_inode=1;
      break;
    }
  }
  //free_dir_listing(names);
  return node;
}

char* fname_for_inode(uint32_t dir_inode_num, uint32_t inode_num,FILE* f,int num) {
  for(int i=0;;i++) {
    ext2_dir_entry* entry=read_dir_entry(dir_inode_num,i,f,num);
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
        blk_grp_num=realloc(blk_grp_num,sizeof(uint32_t)*(max_mnts+32));
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
    uint32_t num_blks=((sizeof(blk_grp)*num_blk_grps)/blk_size[num_mnts])+1;
    blk_grps[num_mnts]=malloc(sizeof(uint8_t)*num_blks*1024);
    blk_grp_num[num_mnts]=num_blk_grps;
    char* data_ptr=(char*)(blk_grps[num_mnts]);
    for (uint32_t i=0;i<num_blks;i++) {
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
      FILE* f=fopen(devs[data->num],"r+");
      uint32_t inode_num=2;
      inode* node;
      for (char* tok=strtok(stream->path,"/");tok!=NULL;tok=strtok(NULL,"/")) {
        char got_inode;
        uint32_t temp_num=inode_for_fname(inode_num,tok,&got_inode,f,data->num);
        if (got_inode) {
          inode_num=temp_num;
          node=read_inode(inode_num,f,data->num);
          if ((node->i_mode&EXT2_S_IFDIR)==0) {
            char* next_tok=strtok(NULL,"/");
            if (next_tok) {
              errno=ENOTDIR;
              fclose(f);
              return 0;
            } else {
              break;
            }
          }
        } else {
          if (stream->wr) {
            char* next_tok=strtok(NULL,"/");
            if (next_tok) {
              errno=ENOENT;
              fclose(f);
              return 0;
            } else {
              klog("INFO","Creating new file");
              klog("INFO","Parent directory inode:%d",inode_num);
              inode_num=reserve_inode(f,data->num);
              klog("INFO","Inode %d is free",inode_num);
              inode node;
              node.i_mode=EXT2_S_IFREG;
              node.i_uid=0;
              node.i_size=0;
              node.i_atime=0;
              node.i_ctime=0;
              node.i_mtime=0;
              node.i_dtime=0;
              node.i_gid=0;
              node.i_links_count=1;
              node.i_blocks=0;
              node.i_flags=0;
              node.i_osd1=0;
              for (int i=0;i<15;i++) {
                node.i_block[i]=0;
              }
              node.i_generation=0;
              node.i_file_acl=0;
              node.i_ext_size_or_dir_acl=0;
              node.i_faddr=0;
              node.i_osd2=0;
              write_inode(inode_num,&node,f,data->num);
              errno=ENOENT;
              fclose(f);
              return 0;
            }
          }
          errno=ENOENT;
          fclose(f);
          return 0;
        }
      }
      data->inode=node;
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
    file_info* data=stream->data;
    FILE* f=fopen(devs[data->num],"r+");
    write_char(data->inode,*((uint8_t*)data1),stream->pos,f,data->num);
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
  blk_grp_num=malloc(sizeof(uint32_t)*32);
  mnts=malloc(sizeof(char*)*32);
  devs=malloc(sizeof(char*)*32);
  num_mnts=0;
  max_mnts=32;
  register_fs(drv,"ext2");
}
