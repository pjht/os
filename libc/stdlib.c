/**
 * \file 
*/

#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCKS 512 //!< Maximum number of blocks that can be used

/**
 * Represents a contiguos block in the heap 
*/
typedef struct {
  char* bitmap; //!< Bitmap of avilable four byte groups
  size_t bitmap_byt_size; //!< Size of the bitmap in bytes
  size_t bitmap_bit_size; //!< Size of the bitmap in bits
  size_t avail_data_size; //!< Size of the data block
  void* data_block; //!< Data block 
} heap_block;


static heap_block entries[MAX_BLOCKS]; //!< List of blocks in the heap
static size_t num_used_entries=0; //!< Number of blocks in the heap

/**
 * Get a bit in a bitmap
 * \param bmap The bitmap
 * \param index The index in the bitmap
 * \return the bit
*/
static char get_bmap_bit(char* bmap,size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  char entry=bmap[byte];
  return (entry&(1<<bit))>0;
}

/**
 * Set a bit in a bitmap
 * \param bmap The bitmap
 * \param index The index in the bitmap
*/
static void set_bmap_bit(char* bmap,size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  bmap[byte]=bmap[byte]|(1<<bit);
}

/**
 * Clear a bit in a bitmap
 * \param bmap The bitmap
 * \param index The index in the bitmap
*/
static void clear_bmap_bit(char* bmap,size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  bmap[byte]=bmap[byte]&(~(1<<bit));
}

/**
 * Add a block to the heap
 * \param mem_blks The number of pages that this block will use
*/
static void reserve_block(size_t mem_blks) {
  size_t bmap_byts=((mem_blks*BLK_SZ)/4)/8;
  entries[num_used_entries].bitmap=alloc_memory((size_t)ceilf((double)bmap_byts/BLK_SZ));
  entries[num_used_entries].bitmap_byt_size=bmap_byts;
  entries[num_used_entries].bitmap_bit_size=bmap_byts*8;
  char* bmap=entries[num_used_entries].bitmap;
  size_t bmap_byt_sz=entries[num_used_entries].bitmap_byt_size;
  for(size_t i=0;i<bmap_byt_sz;i++) {
    bmap[i]=0;
  }
  entries[num_used_entries].avail_data_size=mem_blks*BLK_SZ;
  entries[num_used_entries].data_block=alloc_memory(mem_blks);
  num_used_entries++;
}

void* malloc(size_t size) {
  size_t num_4b_grps=(size_t)ceilf((float)size/4);
  num_4b_grps+=3;
  int blk_indx=-1;
  size_t bmap_index;
  heap_block entry;
  for (size_t i=0;i<num_used_entries;i++) {
    size_t remaining_blks;
    entry=entries[i];
    if (entry.avail_data_size>=size) {
      char* bmap=entry.bitmap;
      size_t bmap_byt_sz=entry.bitmap_byt_size;
      for(size_t i=0;i<bmap_byt_sz;i++) {
        char got_0=0;
        remaining_blks=num_4b_grps;
        size_t old_j;
        for (size_t j=i*8;;j++) {
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
    // reserve_block((size_t)ceilf((double)size/BLK_SZ));
    reserve_block(1024);
    return malloc(size);
  }
  for (size_t i=0;i<num_4b_grps;i++) {
    set_bmap_bit(entry.bitmap,bmap_index+i);
  }
  size_t data_offset=(bmap_index*8)+12;
  size_t* info=(void*)(((char*)entry.data_block)+data_offset-12);
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
  size_t num_4b_grps=*((size_t*)((char*)mem-12));
  memcpy(ptr,mem,num_4b_grps*4);
  //free(mem);
  mem=ptr;
  return ptr;
}

void free(void* mem) {
  size_t* info=(size_t*)((char*)mem-12);
  size_t num_4b_grps=info[0];
  size_t bmap_index=info[1];
  size_t blk_indx=info[2];
  heap_block entry=entries[blk_indx];
  for (size_t i=0;i<num_4b_grps;i++) {
    clear_bmap_bit(entry.bitmap,bmap_index+i);
  }
  entry.avail_data_size+=(num_4b_grps*4)+12;
}
