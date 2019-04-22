#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <memory.h>

#define MAX_BLOCKS 512

typedef struct {
  char* bitmap;
  uint32_t bitmap_byt_size;
  uint32_t bitmap_bit_size;
  uint32_t avail_data_size;
  void* data_block;
} heap_block;


static heap_block entries[MAX_BLOCKS];
static uint32_t num_used_entries=0;

static char get_bmap_bit(char* bmap,uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  char entry=bmap[byte];
  return (entry&(1<<bit))>0;
}

static void set_bmap_bit(char* bmap,uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  bmap[byte]=bmap[byte]|(1<<bit);
}

static void clear_bmap_bit(char* bmap,uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  bmap[byte]=bmap[byte]&(~(1<<bit));
}

static void reserve_block(uint32_t mem_blks) {
  uint32_t bmap_byts=((mem_blks*BLK_SZ)/4)/8;
  entries[num_used_entries].bitmap=alloc_memory((uint32_t)ceilf((double)bmap_byts/BLK_SZ));
  entries[num_used_entries].bitmap_byt_size=bmap_byts;
  entries[num_used_entries].bitmap_bit_size=bmap_byts*8;
  char* bmap=entries[num_used_entries].bitmap;
  uint32_t bmap_byt_sz=entries[num_used_entries].bitmap_byt_size;
  for(uint32_t i=0;i<bmap_byt_sz;i++) {
    bmap[i]=0;
  }
  entries[num_used_entries].avail_data_size=mem_blks*BLK_SZ;
  entries[num_used_entries].data_block=alloc_memory(mem_blks);
  num_used_entries++;
}

void* malloc(size_t size) {
  uint32_t num_4b_grps=(uint32_t)ceilf((float)size/4);
  num_4b_grps+=3;
  int blk_indx=-1;
  uint32_t bmap_index;
  heap_block entry;
  for (uint32_t i=0;i<num_used_entries;i++) {
    uint32_t remaining_blks;
    entry=entries[i];
    if (entry.avail_data_size>=size) {
      char* bmap=entry.bitmap;
      uint32_t bmap_byt_sz=entry.bitmap_byt_size;
      for(uint32_t i=0;i<bmap_byt_sz;i++) {
        if (bmap[i]!=0xFF) {
          char got_0=0;
          remaining_blks=num_4b_grps;
          uint32_t old_j;
          for (uint32_t j=i*8;;j++) {
            char bit=get_bmap_bit(bmap,j);
            if (got_0) {
              if (bit) {
                if (remaining_blks==0) {
                    bmap_index=old_j;
                    break;
                } else {
                  i+=j/8;
                  i--;
                  break;
                }
              } else {
                remaining_blks--;
              }
            } else {
              if (!bit) {
                got_0=1;
                old_j=j;
                remaining_blks--;
              }
            }
            if (remaining_blks==0) {
              bmap_index=old_j;
              break;
            }
          }
        }
        if (remaining_blks==0) {
          break;
        }
      }
    }
    if (remaining_blks==0) {
      blk_indx=i;
      break;
    }
  }
  if (blk_indx==-1) {
    // reserve_block((uint32_t)ceilf((double)size/BLK_SZ));
    reserve_block(256);
    return malloc(size);
  }
  for (uint32_t i=0;i<num_4b_grps;i++) {
    set_bmap_bit(entry.bitmap,bmap_index+i);
  }
  uint32_t data_offset=(bmap_index*8)+12;
  uint32_t* info=(void*)(((char*)entry.data_block)+data_offset-12);
  info[0]=num_4b_grps;
  info[1]=bmap_index;
  info[2]=blk_indx;
  entry.avail_data_size-=size+12;
  return (void*)(((char*)entry.data_block)+data_offset);

}

void* realloc(void *mem, size_t new_sz) {
  void* ptr=malloc(new_sz);
  if (mem==NULL) {
    return ptr;
  }
  uint32_t num_4b_grps=*((uint32_t*)((char*)mem-12));
  memcpy(ptr,mem,num_4b_grps*4);
  free(mem);
  mem=ptr;
  return ptr;
}

void free(void* mem) {
  uint32_t* info=(uint32_t*)((char*)mem-12);
  uint32_t num_4b_grps=info[0];
  uint32_t bmap_index=info[1];
  uint32_t blk_indx=info[2];
  heap_block entry=entries[blk_indx];
  for (uint32_t i=0;i<num_4b_grps;i++) {
    clear_bmap_bit(entry.bitmap,bmap_index+i);
  }
  entry.avail_data_size+=(num_4b_grps*4)+12;
}
